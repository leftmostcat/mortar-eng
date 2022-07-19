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

#include <map>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include "../../../log.hpp"
#include "../../../state.hpp"
#include "../../../streams/memorystream.hpp"
#include "../../material.hpp"
#include "../../mesh.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../../vertex.hpp"
#include "../dds.hpp"
#include "lsw.hpp"

using namespace Mortar::Resource::Providers::LSW;

struct LSWVertexBlock {
  uint32_t size;
  uint32_t id;
  uint32_t offset;
};

struct LSWVertexHeader {
  uint32_t num_vertex_blocks;

  uint32_t unk_0004[3];

  struct LSWVertexBlock *blocks;
};

struct LSWTextureBlockHeader {
  uint32_t offset;

  uint32_t unk_0004[4];
};

struct LSWTextureHeader {
  uint32_t texture_block_offset;
  uint32_t texture_block_size;
  uint32_t num_textures;

  uint32_t unk_000C[4];

  struct LSWTextureBlockHeader *texture_block_headers;
};

struct LSWMaterialHeader {
  uint32_t num_materials;
  uint32_t *material_offsets;
};

struct LSWMaterial {
  uint32_t unk_0000[16];

  uint32_t flags;
  uint32_t moreFlags;

  uint32_t unk_0048[3];

  float red;
  float green;
  float blue;

  uint32_t unk_0060[5];

  float unk_0074;

  int16_t textureIdx;
  uint16_t unk_007A;

  uint32_t unk_007C[2];

  float unk_0084[2];

  uint32_t unk_0090[4];

  uint8_t unk_009C;

  uint8_t effectType;

  uint8_t unk_009E[2];

  uint32_t unk_A0[5];
};

void readMaterial(Stream &stream, LSWMaterial *material) {
  stream.seek(0x40, SEEK_CUR);

  material->flags = stream.readUint32();
  material->moreFlags = stream.readUint32();

  stream.seek(0xc, SEEK_CUR);

  material->red = stream.readFloat();
  material->green = stream.readFloat();
  material->blue = stream.readFloat();

  stream.seek(0x14, SEEK_CUR);

  material->unk_0074 = stream.readFloat();

  int16_t texture_idx = stream.readInt16();

  if (texture_idx != -1 && texture_idx & 0x8000) {
    material->textureIdx = texture_idx & 0x7FFF;
  } else {
    material->textureIdx = texture_idx;
  }

  stream.seek(35, SEEK_CUR);

  material->effectType = stream.readUint8();
}

void LSWProviders::MaterialsProvider::read(std::vector<Material *>& materials, const char *baseName, Stream &stream, uint32_t bodyOffset, const std::vector<Texture *>& textures) {
  ResourceManager resourceManager = State::getResourceManager();

  struct LSWMaterialHeader material_header;

  material_header.num_materials = stream.readUint32();

  if (material_header.num_materials == 0) {
    return;
  }

  material_header.material_offsets = new uint32_t[material_header.num_materials];

  for (int i = 0; i < material_header.num_materials; i++) {
    material_header.material_offsets[i] = stream.readUint32();
  }

  /* Initialize per-model materials, consisting of a color and index to an in-model texture. */
  for (int i = 0; i < material_header.num_materials; i++) {
    // XXX: Breaks if we have more than 99 materials
    char *name = (char *)calloc(strlen(baseName) + 6, sizeof(char));
    sprintf(name, "%s.mat%d", baseName, i);
    Material *material = resourceManager.getResource<Material>(name);

    struct LSWMaterial lswMaterial;

    stream.seek(bodyOffset + material_header.material_offsets[i], SEEK_SET);
    readMaterial(stream, &lswMaterial);

    material->setIsAlphaBlended((lswMaterial.flags & 0xf) != 0);
    material->setIsDynamicallyLit((lswMaterial.flags & 0x30000) != 2);

    material->setColor(
      lswMaterial.red,
      lswMaterial.green,
      lswMaterial.blue);

    if (lswMaterial.textureIdx != -1) {
      material->setTexture(textures.at(lswMaterial.textureIdx));
    }

    materials.push_back(material);
  }

  delete[] material_header.material_offsets;
}

void LSWProviders::TexturesProvider::read(std::vector<Texture *>& textures, const char *baseName, Stream &stream, uint32_t texturesOffset) {
  struct LSWTextureHeader texture_header;

  texture_header.texture_block_offset = stream.readUint32();
  texture_header.texture_block_size = stream.readUint32();
  texture_header.num_textures = stream.readUint32();

  if (texture_header.num_textures == 0) {
    return;
  }

  stream.seek(4 * sizeof(uint32_t), SEEK_CUR);
  texture_header.texture_block_headers = new struct LSWTextureBlockHeader[texture_header.num_textures];

  for (int i = 0; i < texture_header.num_textures; i++) {
    texture_header.texture_block_headers[i].offset = stream.readUint32();

    stream.seek(4 * sizeof(uint32_t), SEEK_CUR);
  }

  /* Read inline DDS textures. */
  for (int i = 0; i < texture_header.num_textures; i++) {
    stream.seek(texturesOffset + texture_header.texture_block_offset + texture_header.texture_block_headers[i].offset, SEEK_SET);

    size_t size;

    /* A rough maximum for file size is calculated from per-texture offsets. */
    if (i < texture_header.num_textures - 1) {
      size = texture_header.texture_block_headers[i + 1].offset - texture_header.texture_block_headers[i].offset;
    } else {
      size = texture_header.texture_block_size - texture_header.texture_block_headers[i].offset;
    }

    uint8_t *texture_data = new uint8_t[size];
    for (int j = 0; j < size; j++) {
      texture_data[j] = stream.readUint8();
    }

    auto textureStream = MemoryStream(texture_data, size);

    // XXX: Breaks if we have more than 99 textures
    char *textureName = (char *)calloc(strlen(baseName) + 6, sizeof(char));
    sprintf(textureName, "%s.tex%.2d", baseName, i);

    Texture *texture = DDSProvider::read(textureName, textureStream);
    textures.push_back(texture);
  }

  delete[] texture_header.texture_block_headers;
}

void LSWProviders::VertexBufferProvider::read(std::vector<VertexBuffer *>& vertexBuffers, const char *baseName, Stream &stream, uint32_t vertexHeaderOffset) {
  struct LSWVertexHeader vertex_header;

  vertex_header.num_vertex_blocks = stream.readUint32();

  if (vertex_header.num_vertex_blocks == 0) {
    return;
  }

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  vertex_header.blocks = new struct LSWVertexBlock[vertex_header.num_vertex_blocks];

  for (int i = 0; i < vertex_header.num_vertex_blocks; i++) {
    vertex_header.blocks[i].size = stream.readUint32();
    vertex_header.blocks[i].id = stream.readUint32();
    vertex_header.blocks[i].offset = stream.readUint32();
  }

  /* Read vertex blocks into individual, indexed buffers. */
   for (int i = 0; i < vertex_header.num_vertex_blocks; i++) {
    // XXX: Breaks if we have more than 99 vertex buffers
    char *vertexBufferName = (char *)calloc(strlen(baseName) + 7, sizeof(char));
    sprintf(vertexBufferName, "%s.vbuf%.2d", baseName, i);

    VertexBuffer *vertexBuffer = State::getResourceManager().getResource<VertexBuffer>(vertexBufferName);
    vertexBuffers.push_back(vertexBuffer);

    vertexBuffer->setSize(vertex_header.blocks[i].size);

    stream.seek(vertexHeaderOffset + vertex_header.blocks[i].offset, SEEK_SET);
    auto data = new uint8_t[vertex_header.blocks[i].size];

    for (int j = 0; j < vertex_header.blocks[i].size; j++) {
      data[j] = stream.readUint8();
    }

    vertexBuffer->setData(data);
  }

  delete[] vertex_header.blocks;
}
