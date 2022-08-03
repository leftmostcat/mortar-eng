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

#include <SDL2/SDL.h>
#include <tsl/sparse_map.h>

#include "../../math/matrix.hpp"
#include "../renderer.hpp"
#include "shader.hpp"

namespace Mortar::Render::GL {
  class Renderer : public Mortar::Render::Renderer {
    public:
      Renderer() :
        d3dTransform { Math::Matrix::diagonal(1.0f, 1.0f, -1.0f) } {};

      void initialize() override;
      void shutDown() override;

      void registerMeshes(const std::vector<Resource::Mesh *>& meshes) override;
      void registerTextures(const std::vector<Resource::Texture *>& textures) override;
      void registerVertexBuffers(const std::vector<Resource::VertexBuffer *>& vertexBuffers) override;

      void renderGeometry(const Resource::GeomObject *geometry) override;

    private:
      ShaderManager shaderManager;
      bool isInitialized;

      tsl::sparse_map<Resource::ResourceHandle, GLuint> elementBufferIds;
      tsl::sparse_map<Resource::ResourceHandle, GLuint> textureIds;
      tsl::sparse_map<Resource::ResourceHandle, GLuint> textureSamplers;
      tsl::sparse_map<Resource::ResourceHandle, GLuint> vertexArrayIds;
      tsl::sparse_map<Resource::ResourceHandle, GLuint> vertexBufferIds;

      const Math::Matrix d3dTransform;
  };
}

#endif
