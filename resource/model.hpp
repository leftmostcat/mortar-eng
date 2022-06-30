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

#include "material.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "resource.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Mortar::Resource {
  class Model : public Resource {
    public:
      Model(ResourceHandle handle)
        : Resource { handle } {};

      void addTexture(Texture *texture);
      const std::vector<Texture *>& getTextures() const;

      void addVertexBuffer(VertexBuffer *vertexBuffer);
      const std::vector<VertexBuffer *>& getVertexBuffers() const;

    private:
      std::vector<Texture *> textures;
      std::vector<VertexBuffer *> vertexBuffers;
  };
}

#endif
