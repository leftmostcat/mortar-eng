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

#include <vector>

#include "filestream.hpp"
#include "hgp.hpp"
#include "log.hpp"
#include "nup.hpp"
#include "glmodel.hpp"
#include "matrix.hpp"
#include "shader.hpp"

#define WIDTH 800
#define HEIGHT 600

void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
  DEBUG("gl error: %s", message);
  exit(-1);
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    DEBUG("failed to initialize SDL");
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow("Mortar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  if (!window) {
    DEBUG("failed to create window");
    SDL_Quit();
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
  glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
  glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);

  EffectManager effectManager = EffectManager();
  if (effectManager.Initialize() == -1) {
    exit(-1);
  }

  /* Read in the specified model. */
  FileStream fs = FileStream(argv[1], "rb");
  Model model;

  if (strcasestr(argv[1], ".hgp")) {
    DEBUG("Loading HGP model %s", argv[1]);
    model = HGPModel(fs);
  }
  else if (strcasestr(argv[1], ".nup")) {
    DEBUG("Loading NUP model %s", argv[1]);
    model = NUPModel(fs);
  }

  GLModel glModel = GLModel(model, &effectManager);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  /* Initialize transformation matrices. */
  glm::mat4 proj = glm::perspective(45.0f, (float)WIDTH / HEIGHT, 0.5f, 10.0f);

  glm::mat4 view = glm::lookAt(
    glm::vec3(0.3f, 0.4f, 0.6f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  glm::mat4 d3dTransform = glm::diagonal4x4(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));

  /* Main loop. */
  bool shouldClose = false;
  while (!shouldClose) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projViewMtx = proj * view * d3dTransform;

    for (int i = 0; i < glModel.renderObjects.size(); i++) {
      GLModel::RenderObject renderObject = glModel.renderObjects[i];

      GLuint shaderProgram = effectManager.GetShaderProgram(renderObject.shaderType);
      glUseProgram(shaderProgram);

      // /* Pull out attribute and uniform locations. */
      GLint tex_unif = glGetUniformLocation(shaderProgram, "materialTex");
      GLint has_tex_unif = glGetUniformLocation(shaderProgram, "hasTexture");

      GLint projViewMtxUnif = glGetUniformLocation(shaderProgram, "projViewMtx");
      GLint mesh_mtx_unif = glGetUniformLocation(shaderProgram, "meshTransformMtx");

      GLint color_unif = glGetUniformLocation(shaderProgram, "materialColor");
      GLint multipliers_unif = glGetUniformLocation(shaderProgram, "colorMultipliers");

      GLint alphaAnimUVUnif = glGetUniformLocation(shaderProgram, "alphaAnimUV");

      // if (renderObject.shaderType == UNLIT) {
      //   glUniform2fv(alphaAnimUVUnif, 1, renderObject.material.alphaAnimUV);
      // }

      glUniformMatrix4fv(projViewMtxUnif, 1, GL_FALSE, glm::value_ptr(projViewMtx));

      float adjustedColor[4];
      memcpy(adjustedColor, renderObject.material.color, 4 * sizeof(float));

      /* Ensure that fragment colors come from the right place. */
      if (renderObject.material.texture_idx != -1) {
        glUniform1i(tex_unif, renderObject.material.texture_idx);
        glUniform1i(has_tex_unif, 1);

        for (int i = 0; i < 3; i++) {
          adjustedColor[i] *= 0.5f;
        }
      }
      else {
        glUniform1i(has_tex_unif, 0);
      }

      float colorMultipliers[2] = {1.0f, 1.0f};
      // if (renderObject.material.flags & Model::Material::USE_VERTEX_COLOR) {
      //   colorMultipliers[0] = 1.0f;
      //   colorMultipliers[1] = 0.0f;
      // }

      glDisable(GL_BLEND);
      glDisable(GL_ALPHA_TEST);

      glUniform2fv(multipliers_unif, 1, colorMultipliers);

      glBindVertexArray(renderObject.vertexArray);

      /* Set per-face material color and transformation matrix. */
      glUniform4fv(color_unif, 1, adjustedColor);
      glUniformMatrix4fv(mesh_mtx_unif, 1, GL_TRUE, glm::value_ptr(renderObject.transformation));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObject.elementBuffer);
      glDrawElements(renderObject.primitiveType, renderObject.elementCount, GL_UNSIGNED_SHORT, 0);
    }

    for (int i = 0; i < glModel.alphaRenderObjects.size(); i++) {
      GLModel::RenderObject renderObject = glModel.alphaRenderObjects[i];

      GLuint shaderProgram = effectManager.GetShaderProgram(renderObject.shaderType);
      glUseProgram(shaderProgram);

      // /* Pull out attribute and uniform locations. */
      GLint tex_unif = glGetUniformLocation(shaderProgram, "materialTex");
      GLint has_tex_unif = glGetUniformLocation(shaderProgram, "hasTexture");

      GLint projViewMtxUnif = glGetUniformLocation(shaderProgram, "projViewMtx");
      GLint mesh_mtx_unif = glGetUniformLocation(shaderProgram, "meshTransformMtx");

      GLint color_unif = glGetUniformLocation(shaderProgram, "materialColor");
      GLint multipliers_unif = glGetUniformLocation(shaderProgram, "colorMultipliers");

      GLint alphaAnimUVUnif = glGetUniformLocation(shaderProgram, "alphaAnimUV");

      // if (renderObject.shaderType == UNLIT) {
      //   glUniform2fv(alphaAnimUVUnif, 1, renderObject.material.alphaAnimUV);
      // }

      glUniformMatrix4fv(projViewMtxUnif, 1, GL_FALSE, glm::value_ptr(projViewMtx));

      float adjustedColor[4];
      memcpy(adjustedColor, renderObject.material.color, 4 * sizeof(float));

      /* Ensure that fragment colors come from the right place. */
      if (renderObject.material.texture_idx != -1) {
        glUniform1i(tex_unif, renderObject.material.texture_idx);
        glUniform1i(has_tex_unif, 1);

        for (int i = 0; i < 3; i++) {
          adjustedColor[i] *= 0.5f;
        }
      }
      else {
        glUniform1i(has_tex_unif, 0);
      }

      float colorMultipliers[2] = {1.0f, 1.0f};
      // if (renderObject.material.flags & Model::Material::USE_VERTEX_COLOR) {
      //   colorMultipliers[0] = 1.0f;
      //   colorMultipliers[1] = 0.0f;
      // }

      glEnable(GL_BLEND);
      glEnable(GL_ALPHA_TEST);
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glAlphaFunc(GL_GEQUAL, (float)((renderObject.material.rawFlags >> 0x17 & 0xff) << 1) / 255.0);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
      glDepthMask(GL_FALSE);

      glUniform2fv(multipliers_unif, 1, colorMultipliers);

      glBindVertexArray(renderObject.vertexArray);

      /* Set per-face material color and transformation matrix. */
      glUniform4fv(color_unif, 1, adjustedColor);
      glUniformMatrix4fv(mesh_mtx_unif, 1, GL_TRUE, glm::value_ptr(renderObject.transformation));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObject.elementBuffer);
      glDrawElements(renderObject.primitiveType, renderObject.elementCount, GL_UNSIGNED_SHORT, 0);

      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDepthMask(GL_TRUE);
    }

    SDL_GL_SwapWindow(window);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)) {
        shouldClose = true;
      }
    }
  }

  SDL_Quit();

  return 0;
}
