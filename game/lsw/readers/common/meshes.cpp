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

#include "../../../../log.hpp"
#include "../../../../state.hpp"
#include "../../../../resource/mesh.hpp"
#include "../common.hpp"

using namespace Mortar::Game::LSW::Readers;

struct LSWMeshHeader {
  uint32_t unk_0000[3];

  uint32_t mesh_offset;

  uint32_t unk_0010;
};

struct LSWMesh {
  uint32_t next_offset;

  uint32_t unk_0004;

  uint32_t materialIdx;
  uint32_t vertexType;

  uint32_t unk_0010[3];

  uint32_t vertexBlockIdx;

  uint32_t unk_0020;

  uint32_t unk_0024;

  uint32_t unk_0028[2];

  uint32_t surfacesOffset;

  uint32_t unk_0034;

  uint32_t unk_0038;
  uint32_t unk_003C;
};

struct LSWSurface {
  uint32_t next_offset;
  uint32_t primitiveType;

  uint16_t elementCount;
  uint16_t unk_000A;

  uint32_t elementsOffset;

  uint32_t unk_0010;

  uint8_t num_skin_matrices;

  uint8_t unk_0015;

  uint16_t skin_matrix_indices[16];

  uint16_t unk_0036;
  uint32_t unk_0038;
  uint32_t unk_003C;
  uint32_t unk_0040;
  uint32_t unk_0044;
  uint32_t unk_0048;
  uint32_t unk_004C;
};

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

const Mortar::Resource::VertexLayout& getVertexLayoutFromMesh(LSWMesh& mesh) {
  if (!vertexLayouts.contains(mesh.vertexType)) {
    throw std::runtime_error("unimplemented vertex layout");
  }

  return vertexLayouts.at(mesh.vertexType);
}

Mortar::Resource::ShaderType getShaderTypeFromMesh(LSWMesh& mesh, const Mortar::Resource::Material *material) {
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

const struct LSWSurface readSurfaceInfo(Stream &stream, const uint32_t bodyOffset, uint32_t surfaceOffset) {
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

void processSurfaces(Stream &stream, const uint32_t bodyOffset, uint32_t surfacesOffset, Mortar::Resource::Mesh *mesh) {
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

const struct LSWMesh readMeshInfo(Stream &stream, const uint32_t body_offset, uint32_t mesh_offset) {
  stream.seek(body_offset + mesh_offset, SEEK_SET);
  struct LSWMesh mesh;

  mesh.next_offset = stream.readUint32();

  stream.seek(1 * sizeof(uint32_t), SEEK_CUR);

  mesh.materialIdx = stream.readUint32();
  mesh.vertexType = stream.readUint32();

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  mesh.vertexBlockIdx = stream.readUint32();

  stream.seek(1 * sizeof(uint32_t), SEEK_CUR);

  mesh.unk_0024 = stream.readUint32();

  stream.seek(2 * sizeof(uint32_t), SEEK_CUR);

  mesh.surfacesOffset = stream.readUint32();

  stream.seek(4, SEEK_CUR);

  mesh.unk_0038 = stream.readUint32();
  mesh.unk_003C = stream.readUint32();

  return mesh;
}

void MeshesReader::read(std::vector<Resource::Mesh *>& meshes, Stream &stream, uint32_t bodyOffset, const std::vector<Resource::Material *>& materials, const std::vector<Resource::VertexBuffer *>& vertexBuffers) {
  Resource::ResourceManager resourceManager = State::getResourceManager();

  struct LSWMeshHeader mesh_header;

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  mesh_header.mesh_offset = stream.readUint32();

  if (!mesh_header.mesh_offset) {
    return;
  }

  uint32_t nextOffset = mesh_header.mesh_offset;
  do {
    struct LSWMesh lswMesh = readMeshInfo(stream, bodyOffset, nextOffset);

    Resource::Mesh *mesh = resourceManager.createResource<Resource::Mesh>();
    meshes.push_back(mesh);

    Resource::Material *material = materials.at(lswMesh.materialIdx);
    mesh->setMaterial(material);

    Resource::ShaderType shaderType = getShaderTypeFromMesh(lswMesh, material);
    mesh->setShaderType(shaderType);

    const Resource::VertexLayout& vertexLayout = getVertexLayoutFromMesh(lswMesh);
    mesh->setVertexLayout(vertexLayout);

    Resource::VertexBuffer *vertexBuffer = vertexBuffers.at(lswMesh.vertexBlockIdx - 1);
    mesh->setVertexBuffer(vertexBuffer);

    processSurfaces(stream, bodyOffset, lswMesh.surfacesOffset, mesh);

    nextOffset = lswMesh.next_offset;
  } while (nextOffset);
}
