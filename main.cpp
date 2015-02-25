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
#include "hgp.hpp"
#include "matrix.hpp"

#define GLSL(src) "#version 130\n" #src

#define WIDTH 800
#define HEIGHT 600

const GLchar *vertex_source = GLSL(
	uniform mat4 projectionMtx;
	uniform mat4 viewMtx;
	uniform mat4 modelMtx;
	uniform mat4 meshTransformMtx;

	in vec3 position;
	in vec2 texCoord;

	out vec2 fragTexCoord;

	void main()
	{
		fragTexCoord = texCoord;
		gl_Position = projectionMtx * viewMtx * modelMtx * meshTransformMtx * vec4(position, 1.0);
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
	if (argc < 2 || access(argv[1], R_OK) != 0) {
		fprintf(stdout, "%s: please specify an HGP file to open\n", argv[0]);
		return -1;
	}

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Mortar Engine", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	/* Compile and link shaders. */
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(fragment_shader);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glBindFragDataLocation(shader_program, 0, "outColor");
	glLinkProgram(shader_program);

	glUseProgram(shader_program);

	/* Pull out attribute and uniform locations. */
	GLint color_unif = glGetUniformLocation(shader_program, "materialColor");
	GLint tex_unif = glGetUniformLocation(shader_program, "materialTex");
	GLint has_tex_unif = glGetUniformLocation(shader_program, "hasTexture");

	GLint projection_mtx_unif = glGetUniformLocation(shader_program, "projectionMtx");
	GLint view_mtx_unif = glGetUniformLocation(shader_program, "viewMtx");
	GLint model_mtx_unif = glGetUniformLocation(shader_program, "modelMtx");
	GLint mesh_mtx_unif = glGetUniformLocation(shader_program, "meshTransformMtx");

	GLint position_attr = glGetAttribLocation(shader_program, "position");
	GLint texcoord_attr = glGetAttribLocation(shader_program, "texCoord");

	/* Read in the specified HGP model. */
	Model hgp = HGPModel::HGPModel(argv[1]);

	GLuint *vao = new GLuint[hgp.getVertexBufferCount()];
	GLuint *vbo = new GLuint[hgp.getVertexBufferCount()];

	glGenVertexArrays(hgp.getVertexBufferCount(), vao);
	glGenBuffers(hgp.getVertexBufferCount(), vbo);

	/* Initialize all vertex buffers. */
	for (int i = 0; i < hgp.getVertexBufferCount(); i++) {
		glBindVertexArray(vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);

		Model::VertexBuffer vertexBuffer = hgp.getVertexBuffer(i);

		glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size, vertexBuffer.data, GL_STATIC_DRAW);

		glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, vertexBuffer.stride, NULL);
		glEnableVertexAttribArray(position_attr);

		glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE, vertexBuffer.stride, (GLvoid *)28);
		glEnableVertexAttribArray(texcoord_attr);
	}

	/* Initialize GL texture objects. */
	GLuint *tex = new GLuint[hgp.getTextureCount()];

	glGenTextures(hgp.getTextureCount(), tex);

	for (int i = 0; i < hgp.getTextureCount(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]);

		Texture texture = hgp.getTexture(i);

		if (texture.compressed)
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, texture.internal_format, texture.width, texture.height, 0, texture.levels[0].size, texture.levels[0].data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	/* Initialize an element buffer for each chunk. */
	GLuint *ebo = new GLuint[hgp.getChunkCount()];

	glGenBuffers(hgp.getChunkCount(), ebo);

	for (int i = 0; i < hgp.getChunkCount(); i++) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);

		Model::Chunk chunk = hgp.getChunk(i);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * chunk.num_elements, chunk.element_buffer, GL_STATIC_DRAW);
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Initialize transformation matrices. */
	glm::mat4 proj = glm::perspective(45.0f, (float)WIDTH / HEIGHT, 1.0f, 10.0f);
	glUniformMatrix4fv(projection_mtx_unif, 1, GL_FALSE, glm::value_ptr(proj));

	glm::mat4 view = glm::lookAt(
		glm::vec3(1.0f, 0.5f, -0.8f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	glUniformMatrix4fv(view_mtx_unif, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 model = glm::mat4();
	glUniformMatrix4fv(model_mtx_unif, 1, GL_FALSE, glm::value_ptr(model));

	/* Main loop. */
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < hgp.getChunkCount(); i++) {
			Model::Chunk chunk = hgp.getChunk(i);

			/* Get chunk-specific information for rendering. */
			Model::Material material = hgp.getMaterial(chunk.material_idx);
			GLenum prim_type = getGLPrimitiveType(chunk.primitive_type);

			int vb_idx = chunk.vertex_buffer_idx;

			/* Ensure that fragment colors come from the right place. */
			if (material.texture_idx != -1) {
				glUniform1i(tex_unif, material.texture_idx);
				glUniform1i(has_tex_unif, 1);
			}
			else {
				glUniform1i(has_tex_unif, 0);
			}

			/* Set per-chunk material color and transformation matrix. */
			glUniform4fv(color_unif, 1, material.color);
			glUniformMatrix4fv(mesh_mtx_unif, 1, GL_FALSE, chunk.transformation.array16);

			/* Render the chunk. */
			glBindVertexArray(vao[vb_idx]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
			glDrawElements(prim_type, chunk.num_elements, GL_UNSIGNED_SHORT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	/* Final cleanup. */
	glDeleteProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glDeleteTextures(hgp.getTextureCount(), tex);
	glDeleteBuffers(hgp.getChunkCount(), ebo);
	glDeleteBuffers(hgp.getVertexBufferCount(), vbo);
	glDeleteVertexArrays(hgp.getVertexBufferCount(), vao);

	delete vao;
	delete vbo;
	delete ebo;
	delete tex;

	glfwTerminate();

	return 0;
}
