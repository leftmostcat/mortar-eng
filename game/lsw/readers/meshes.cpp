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

#include <stdexcept>
#include <tsl/sparse_map.h>

#include "../../../log.hpp"
#include "../../../state.hpp"
#include "../../../resource/mesh.hpp"
#include "common.hpp"

using namespace Mortar::Game::LSW::Readers;

static tsl::sparse_map<uint32_t, Mortar::Resource::VertexLayout> vertexLayouts {
  {
    0x59, {
      36,
      {
        { Mortar::Resource::VertexUsage::POSITION,  Mortar::Resource::VertexDataType::VEC3,     0x0  },
        { Mortar::Resource::VertexUsage::NORMAL,    Mortar::Resource::VertexDataType::VEC3,     0xc  },
        { Mortar::Resource::VertexUsage::COLOR,     Mortar::Resource::VertexDataType::D3DCOLOR, 0x18 },
        { Mortar::Resource::VertexUsage::TEX_COORD, Mortar::Resource::VertexDataType::VEC2,     0x1c },
      }
    }
  },
  {
    0x5d, {
      56,
      {
        { Mortar::Resource::VertexUsage::POSITION,      Mortar::Resource::VertexDataType::VEC3,     0x0  },
        { Mortar::Resource::VertexUsage::BLEND_WEIGHTS, Mortar::Resource::VertexDataType::VEC2,     0xc  },
        { Mortar::Resource::VertexUsage::BLEND_INDICES, Mortar::Resource::VertexDataType::VEC3,     0x14 },
        { Mortar::Resource::VertexUsage::NORMAL,        Mortar::Resource::VertexDataType::VEC3,     0x20 },
        { Mortar::Resource::VertexUsage::COLOR,         Mortar::Resource::VertexDataType::D3DCOLOR, 0x2c },
        { Mortar::Resource::VertexUsage::TEX_COORD,     Mortar::Resource::VertexDataType::VEC2,     0x30 },
      }
    }
  },
};

const Mortar::Resource::VertexLayout& CommonReaders::getVertexLayoutFromMesh(LSWMesh& mesh) {
  if (!vertexLayouts.contains(mesh.vertexType)) {
    throw std::runtime_error("unimplemented vertex layout");
  }

  return vertexLayouts.at(mesh.vertexType);
}

Mortar::Resource::ShaderType CommonReaders::getShaderTypeFromMesh(LSWMesh& mesh, const Mortar::Resource::Material *material) {
  bool skinned = mesh.vertexType == 0x5d || mesh.unk_0038 != 0;
  bool blended = mesh.unk_0024 != 0 && mesh.unk_003C != 0;

  if (skinned) {
    if (blended) {
      DEBUG("WARNING: blended, skinned geometry is not implemented");

      return Mortar::Resource::ShaderType::INVALID;
    }

    // Unblended, skinned geometry
    return Mortar::Resource::ShaderType::SKIN;
  } else if (material->isDynamicallyLit()) {
    return Mortar::Resource::ShaderType::BASIC;
  } else {
    switch (mesh.vertexType) {
      case 0x59:
        return Mortar::Resource::ShaderType::UNLIT;
        break;
      default:
        DEBUG("unimplemented vertex type");
        break;
    }
  }

  return Mortar::Resource::ShaderType::INVALID;
}

const struct CommonReaders::LSWSurface CommonReaders::readSurfaceInfo(Stream &stream, const uint32_t bodyOffset, uint32_t surfaceOffset) {
  stream.seek(bodyOffset + surfaceOffset, SEEK_SET);
  struct LSWSurface surface;

  surface.next_offset = stream.readUint32();
  surface.primitiveType = stream.readUint32();

  surface.elementCount = stream.readUint16();

  stream.seek(sizeof(uint16_t), SEEK_CUR);

  surface.elementsOffset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);

	surface.num_skin_matrices = stream.readUint8();

	stream.seek(sizeof(uint8_t), SEEK_CUR);

  for (int i = 0; i < 16; i++) {
    surface.skin_matrix_indices[i] = stream.readUint16();
  }

  // We can safely skip reading the rest of the unknown fields as we'll seek to
  // the next surface or elsewhere after this

  return surface;
}

tsl::sparse_map<uint32_t, Mortar::Resource::PrimitiveType> primitiveTypes {
  { 1, Mortar::Resource::PrimitiveType::LINE_LIST },
  { 2, Mortar::Resource::PrimitiveType::TRIANGLE_LIST },
  { 3, Mortar::Resource::PrimitiveType::TRIANGLE_STRIP },
  { 4, Mortar::Resource::PrimitiveType::LINE_LIST },
  { 5, Mortar::Resource::PrimitiveType::TRIANGLE_LIST },
  { 6, Mortar::Resource::PrimitiveType::TRIANGLE_STRIP },
};

void CommonReaders::processSurfaces(Stream &stream, const uint32_t bodyOffset, uint32_t surfacesOffset, Mortar::Resource::Mesh *mesh) {
  Mortar::Resource::ResourceManager resourceManager = Mortar::State::getResourceManager();

  uint32_t nextOffset = surfacesOffset;
  unsigned i = 0;
  do {
    Mortar::Resource::Surface *surface = resourceManager.createResource<Mortar::Resource::Surface>();
    mesh->addSurface(surface);

    struct LSWSurface lswSurface = readSurfaceInfo(stream, bodyOffset, nextOffset);

    stream.seek(bodyOffset + lswSurface.elementsOffset, SEEK_SET);

    uint16_t *elementData = new uint16_t[lswSurface.elementCount];
    for (int i = 0; i < lswSurface.elementCount; i++) {
      elementData[i] = stream.readUint16();
    }

    Mortar::Resource::IndexBuffer *indexBuffer = resourceManager.createResource<Mortar::Resource::IndexBuffer>();

    indexBuffer->setCount(lswSurface.elementCount);
    indexBuffer->setData(elementData);

    surface->setIndexBuffer(indexBuffer);

    surface->setPrimitiveType(primitiveTypes.at(lswSurface.primitiveType));

    surface->setSkinTransformCount(lswSurface.num_skin_matrices);

    std::vector<ushort> indices(std::begin(lswSurface.skin_matrix_indices), std::end(lswSurface.skin_matrix_indices));
    surface->setSkinTransformIndices(indices);

    nextOffset = lswSurface.next_offset;
    i++;
  } while (nextOffset);
}