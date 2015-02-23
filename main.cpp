/* This file is part of mortar.
 *
 * mortar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mortar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with mortar.  If not, see <http://www.gnu.org/licenses/>.
 */

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hgp.h"

#define GLSL(src) "#version 130\n" #src

#define WIDTH 800
#define HEIGHT 800

const GLchar *vertex_source = GLSL(
	uniform mat4 perspectiveMtx;
	uniform mat4 meshTransformMtx;

	in vec3 position;
	in vec2 texCoord;

	out vec2 fragTexCoord;

	void main()
	{
		vec4 transformedPos = meshTransformMtx * vec4(position, 1.0);

		fragTexCoord = texCoord;
		gl_Position = transformedPos;
	}
);

const GLchar *fragment_source = GLSL(
	uniform vec4 materialColor;
	uniform sampler2D materialTex;
	uniform int hasTexture;

	in vec2 fragTexCoord;

	out vec4 outColor;

	void main()
	{
		vec4 texColor = texture(materialTex, fragTexCoord);

		outColor = mix(materialColor, texColor, hasTexture);
	}
);

static GLenum getGLPrimitiveType(int serial) {
	GLenum prim_type = GL_POINTS;

	switch (serial) {
		case 1:
		case 4:
			prim_type = GL_LINES;
			break;
		case 2:
		case 5:
			prim_type = GL_TRIANGLES;
			break;
		case 3:
		case 6:
			prim_type = GL_TRIANGLE_STRIP;
			break;
	}

	return prim_type;
}

int main(int argc, char **argv) {
	GLFWwindow *window;
	Model *hgp;
	GLuint *vao, *vbo, *ebo, *tex;
	GLuint vertex_shader, fragment_shader, shader_program;
	GLint position_attr, texcoord_attr, color_attr, tex_attr, mix_attr, matrix_attr;

	if (argc < 2 || access(argv[1], R_OK) != 0) {
		fprintf(stdout, "%s: please specify an HGP file to open\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Mortar Engine", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	/* Compile and link shaders. */
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(fragment_shader);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glBindFragDataLocation(shader_program, 0, "outColor");
	glLinkProgram(shader_program);

	glUseProgram(shader_program);

	color_attr = glGetUniformLocation(shader_program, "materialColor");
	tex_attr = glGetUniformLocation(shader_program, "materialTex");
	mix_attr = glGetUniformLocation(shader_program, "hasTexture");
	matrix_attr = glGetUniformLocation(shader_program, "meshTransformMtx");

	/* Read in the specified HGP model. */
	hgp = new HGPModel();
	hgp->load(argv[1]);

	vao = (GLuint *)calloc(hgp->num_vertex_buffers, sizeof(GLuint));
	vbo = (GLuint *)calloc(hgp->num_vertex_buffers, sizeof(GLuint));

	glGenVertexArrays(hgp->num_vertex_buffers, vao);
	glGenBuffers(hgp->num_vertex_buffers, vbo);

	/* Initialize all vertex buffers. */
	for (int i = 0; i < hgp->num_vertex_buffers; i++) {
		glBindVertexArray(vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);

		glBufferData(GL_ARRAY_BUFFER, hgp->vertex_buffers[i].size, hgp->vertex_buffers[i].ptr, GL_STATIC_DRAW);

		position_attr = glGetAttribLocation(shader_program, "position");
		glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, hgp->vertex_buffers[i].stride, NULL);
		glEnableVertexAttribArray(position_attr);

		texcoord_attr = glGetAttribLocation(shader_program, "texCoord");
		glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE, hgp->vertex_buffers[i].stride, (GLvoid *)28);
		glEnableVertexAttribArray(texcoord_attr);
	}

	tex = (GLuint *)calloc(hgp->num_textures, sizeof(GLuint));

	glGenTextures(hgp->num_textures, tex);

	for (int i = 0; i < hgp->num_textures; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]);

		if (hgp->textures[i]->compressed)
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, hgp->textures[i]->internal_format, hgp->textures[i]->width, hgp->textures[i]->height, 0, hgp->textures[i]->levels[0].size, hgp->textures[i]->levels[0].data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	ebo = (GLuint *)calloc(hgp->num_chunks, sizeof(GLuint));

	glGenBuffers(hgp->num_chunks, ebo);

	/* Initialize an element buffer for each mesh. */
	for (int i = 0; i < hgp->num_chunks; i++) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * hgp->chunks[i].num_elements, hgp->chunks[i].element_buffer, GL_STATIC_DRAW);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glFrontFace(GL_CW);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < hgp->num_chunks; i++) {
			/* Get chunk-specific information for rendering. */
			int vb_idx = hgp->chunks[i].vertex_buffer_idx;
			int mat_idx = hgp->chunks[i].material_idx;
			GLenum prim_type = getGLPrimitiveType(hgp->chunks[i].primitive_type);

			if (hgp->materials[mat_idx].texture_idx != -1) {
				glUniform1i(tex_attr, hgp->materials[mat_idx].texture_idx);
				glUniform1i(mix_attr, 1);
			}
			else {
				glUniform1i(mix_attr, 0);
			}

			glUniform4f(color_attr, hgp->materials[mat_idx].red, hgp->materials[mat_idx].green, hgp->materials[mat_idx].blue, hgp->materials[mat_idx].alpha);
			glUniformMatrix4fv(matrix_attr, 1, GL_FALSE, hgp->chunks[i].transformation.array16);

			/* Render the chunk. */
			glBindVertexArray(vao[vb_idx]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
			glDrawElements(prim_type, hgp->chunks[i].num_elements, GL_UNSIGNED_SHORT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	glDeleteProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glDeleteTextures(hgp->num_textures, tex);
	glDeleteBuffers(hgp->num_chunks, ebo);
	glDeleteBuffers(hgp->num_vertex_buffers, vbo);
	glDeleteVertexArrays(hgp->num_vertex_buffers, vao);

	delete hgp;
	glfwTerminate();

	return 0;
}
