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

#ifndef MORTAR_RENDER_RENDERER_H
#define MORTAR_RENDER_RENDERER_H

#include <list>
#include <vector>

#include "../resource/geom.hpp"
#include "../resource/mesh.hpp"

namespace Mortar::Render {
  class Renderer {
    public:
      virtual void initialize() = 0;
      virtual void shutDown() = 0;

      virtual void registerMeshes(const std::vector<const Resource::Mesh *>& meshes) = 0;
      virtual void registerTextures(const std::vector<const Resource::Texture *>& textures) = 0;
      virtual void registerVertexBuffers(const std::vector<const Resource::VertexBuffer *>& vertexBuffers) = 0;

      virtual void renderGeometry(const std::list<const Resource::GeomObject *>& geometry) = 0;
  };
}

#endif
