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

#ifndef MORTAR_RENDER_GL_SHADER_H
#define MORTAR_RENDER_GL_SHADER_H

#include <GL/gl.h>
#include <vector>

#include "../../resource/types/shader.hpp"

namespace Mortar::Render::GL {
  class ShaderManager {
    public:
      ShaderManager();

      void initialize();
      void shutDown();

      GLuint getShaderProgram(Mortar::Resource::ShaderType shaderType);

    private:
      class ShaderProgram {
        public:
          ShaderProgram()
            : program { -1 },
              vertexShader { -1 },
              fragmentShader { -1 } {};

          void initialize(const char *vertexShaderSrc, const char *fragmentShaderSrc);
          void shutDown();

          GLuint getShaderProgram();

        private:
          GLint program;
          GLint vertexShader;
          GLint fragmentShader;
      };

      std::vector<ShaderProgram *> shaderPrograms;
};
}

#endif
