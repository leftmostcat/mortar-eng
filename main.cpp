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
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "filestream.hpp"
#include "hgp.hpp"
#include "nup.hpp"
#include "glmodel.hpp"
#include "matrix.hpp"

#define GLSL(src) "#version 130\n" #src

#define WIDTH 800
#define HEIGHT 600

const GLchar *vertex_source = GLSL(
	uniform mat4 projectionMtx;
	uniform mat4 viewMtx;
	uniform mat4 meshTransformMtx;

	in vec3 position;
	in vec2 texCoord;

	out vec2 fragTexCoord;

	void main()
	{
		fragTexCoord = texCoord;
		gl_Position = projectionMtx * viewMtx * meshTransformMtx * vec4(position, 1.0);
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

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stdout, "%s: please specify a model file to open\n", argv[0]);
		return -1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)) {
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow("Mortar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	if (!window) {
		SDL_Quit();
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GLContext context = SDL_GL_CreateContext(window);

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
	GLint mesh_mtx_unif = glGetUniformLocation(shader_program, "meshTransformMtx");

	/* Read in the specified model. */
	FileStream fs = FileStream(argv[1], "rb");
	Model model;

	if (strcasestr(argv[1], ".hgp"))
		model = HGPModel(fs);
	else if (strcasestr(argv[1], ".nup"))
		model = NUPModel(fs);

	GLModel glModel = GLModel(model, shader_program);

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

	/* Main loop. */
	bool shouldClose = false;
	while (!shouldClose) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < glModel.renderObjects.size(); i++) {
			GLModel::RenderObject renderObject = glModel.renderObjects[i];

			/* Ensure that fragment colors come from the right place. */
			if (renderObject.material.texture_idx != -1) {
				glUniform1i(tex_unif, renderObject.material.texture_idx);
				glUniform1i(has_tex_unif, 1);
			}
			else {
				glUniform1i(has_tex_unif, 0);
			}

			glBindVertexArray(renderObject.vertexArray);

			/* Set per-chunk material color and transformation matrix. */
			glUniform4fv(color_unif, 1, renderObject.material.color);
			glUniformMatrix4fv(mesh_mtx_unif, 1, GL_TRUE, glm::value_ptr(renderObject.transformation));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObject.elementBuffer);
			glDrawElements(renderObject.primitiveType, renderObject.elementCount, GL_UNSIGNED_SHORT, 0);
		}

		SDL_GL_SwapWindow(window);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)) {
				shouldClose = true;
			}
		}
	}

	/* Final cleanup. */
	glDeleteProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	SDL_Quit();

	return 0;
}
