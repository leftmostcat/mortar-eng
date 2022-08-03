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

#ifndef MORTAR_LSW_READERS_COMMON_H
#define MORTAR_LSW_READERS_COMMON_H

#include <cstdint>

#include "../../../state.hpp"
#include "../../../streams/stream.hpp"
#include "../../../resource/types/material.hpp"
#include "../../../resource/types/mesh.hpp"
#include "../../../resource/types/texture.hpp"
#include "../../../resource/types/vertex.hpp"

namespace Mortar::Game::LSW::Readers {
  class MaterialsReader {
    public:
      static void read(std::vector<Resource::Material *>& materials, Stream& stream, uint32_t bodyOffset, const std::vector<Resource::Texture *>& textures);
  };

  class TexturesReader {
    public:
      static void read(std::vector<Resource::Texture *>& textures, Stream& stream, uint32_t texturesOffset);
  };

  class VertexBufferReader {
    public:
      static void read(std::vector<Resource::VertexBuffer *>& vertexBuffers, Stream& stream, uint32_t bodyOffset);
  };

  class MeshesReader {
    public:
      static void read(std::vector<Resource::Mesh *>& meshes, Stream& stream, uint32_t bodyOffset, const std::vector<Resource::Material *>& materials, const std::vector<Resource::VertexBuffer *>& vertexBuffers);
  };
}

#endif
