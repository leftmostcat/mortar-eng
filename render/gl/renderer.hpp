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

#ifndef MORTAR_RENDER_GL_RENDERER_H
#define MORTAR_RENDER_GL_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <SDL2/SDL_video.h>
#include <map>

#include "../renderer.hpp"
#include "shader.hpp"

namespace Mortar::Render::GL {
  class Renderer : public Mortar::Render::Renderer {
    public:
      Renderer(SDL_Window *window) :
        Mortar::Render::Renderer { window },
        d3dTransform { glm::diagonal4x4(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f)) } {};

      virtual void initialize() override;
      virtual void shutDown() override;

      virtual void registerMeshes(const std::vector<Resource::Mesh *>& meshes) override;
      virtual void registerTextures(const std::vector<Resource::Texture *>& textures) override;
      virtual void registerVertexBuffers(const std::vector<Resource::VertexBuffer *>& vertexBuffers) override;

      virtual void renderGeometry(const Resource::GeomObject *geometry) override;

    private:
      ShaderManager shaderManager;
      bool isInitialized;

      std::map<Resource::ResourceHandle, GLuint> elementBufferIds;
      std::map<Resource::ResourceHandle, GLuint> textureIds;
      std::map<Resource::ResourceHandle, GLuint> textureSamplers;
      std::map<Resource::ResourceHandle, GLuint> vertexArrayIds;
      std::map<Resource::ResourceHandle, GLuint> vertexBufferIds;

      const glm::mat4 d3dTransform;
  };
}

#endif
