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

#ifndef MORTAR_MODEL_H
#define MORTAR_MODEL_H

#include <vector>
#include <stdint.h>

#include "../resource.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Mortar::Resource {
  class Model : public Resource {
    public:
      Model(ResourceHandle handle)
        : Resource { handle } {};

      void addMesh(const Mesh *mesh);
      const std::vector<const Mesh *>& getMeshes() const;

      void addTexture(const Texture *texture);
      const std::vector<const Texture *>& getTextures() const;

      void addVertexBuffer(const VertexBuffer *vertexBuffer);
      const std::vector<const VertexBuffer *>& getVertexBuffers() const;

    private:
      std::vector<const Mesh *> meshes;
      std::vector<const Texture *> textures;
      std::vector<const VertexBuffer *> vertexBuffers;
  };
}

#endif
