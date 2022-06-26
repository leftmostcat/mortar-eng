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

#ifndef MORTAR_LSW_H
#define MORTAR_LSW_H

#include <cstdint>
#include <stdint.h>

#include "model.hpp"
#include "stream.hpp"

namespace Mortar {
  namespace LSW {
    struct VertexBlock {
      uint32_t size;
      uint32_t id;
      uint32_t offset;
    };

    struct VertexHeader {
      uint32_t num_vertex_blocks;

      uint32_t unk_0004[3];

      struct VertexBlock *blocks;
    };

    struct TextureBlockHeader {
      uint32_t offset;

      uint32_t unk_0004[4];
    };

    struct TextureHeader {
      uint32_t texture_block_offset;
      uint32_t texture_block_size;
      uint32_t num_textures;

      uint32_t unk_000C[4];

      struct TextureBlockHeader *texture_block_headers;
    };

    struct LayerHeader {
      uint32_t name_offset;
      uint32_t mesh_header_list_offsets[4];
    };

    struct MeshHeader {
      uint32_t unk_0000[3];

      uint32_t mesh_offset;

      uint32_t unk_0010;
    };

    struct Mesh {
      uint32_t next_offset;

      uint32_t unk_0004;

      uint32_t material_idx;
      uint32_t vertex_type;

      uint32_t unk_0010[3];

      uint32_t vertex_block_idx;

      uint32_t unk_0020;

      uint32_t unk_0024;

      uint32_t unk_0028[2];

      uint32_t face_offset;

      uint32_t unk_0034;

      uint32_t unk_0038;
      uint32_t unk_003C;
    };

    struct Face {
      uint32_t next_offset;
      uint32_t primitive_type;

      uint16_t num_elements;
      uint16_t unk_000A;

      uint32_t elements_offset;

      uint32_t unk_0010[16];
    };

    struct MaterialHeader {
      uint32_t num_materials;
      uint32_t *material_offsets;
    };

    struct Material {
      uint32_t unk_0000[16];

      uint32_t flags;
      uint32_t moreFlags;

      uint32_t unk_0048[3];

      float red;
      float green;
      float blue;

      uint32_t unk_0060[5];

      uint32_t alpha;

      int16_t texture_idx;
      uint16_t unk_007A;

      uint32_t unk_007C[2];

      float unk_0084[2];

      uint32_t unk_0090[10];
    };

    std::vector<Model::Mesh> processMeshHeader(Stream &stream, const uint32_t body_offset, uint32_t mesh_header_offset, std::vector<Model::VertexBuffer> &vertexBuffers);
  }
}

#endif
