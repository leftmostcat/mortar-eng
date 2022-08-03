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

#ifndef MORTAR_RESOURCE_PROVIDERS_LSW_H
#define MORTAR_RESOURCE_PROVIDERS_LSW_H

#include <cstdint>
#include <stdint.h>

#include "../../../state.hpp"
#include "../../../streams/stream.hpp"
#include "../../material.hpp"
#include "../../mesh.hpp"
#include "../../texture.hpp"
#include "../../vertex.hpp"

namespace Mortar::Resource::Providers::LSW {
  class LSWProviders {
    public:
      class MaterialsProvider {
        public:
          static void read(std::vector<Material *>& materials, Stream& stream, uint32_t bodyOffset, const std::vector<Texture *>& textures);
      };

      class TexturesProvider {
        public:
          static void read(std::vector<Texture *>& textures, Stream& stream, uint32_t texturesOffset);
      };

      class VertexBufferProvider {
        public:
          static void read(std::vector<VertexBuffer *>& vertexBuffers, Stream& stream, uint32_t bodyOffset);
      };

      class MeshesProvider {
        public:
          template <class T>
          static void read(std::vector<T *>& meshes, Stream& stream, uint32_t bodyOffset, const std::vector<Material *>& materials, const std::vector<VertexBuffer *>& vertexBuffers);
      };

    private:
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

      static Mortar::Resource::ShaderType getShaderTypeFromMesh(LSWMesh& mesh, const Mortar::Resource::Material *material);
      static const Mortar::Resource::VertexLayout& getVertexLayoutFromMesh(LSWMesh& mesh);
      static void processSurfaces(Stream &stream, const uint32_t bodyOffset, uint32_t surfacesOffset, Mortar::Resource::Mesh *mesh);
      static const struct LSWMesh readMeshInfo(Stream &stream, const uint32_t body_offset, uint32_t mesh_offset);
      static const struct LSWSurface readSurfaceInfo(Stream &stream, const uint32_t bodyOffset, uint32_t surfaceOffset);
  };

  template <class T> void LSWProviders::MeshesProvider::read(std::vector<T *>& meshes, Stream &stream, uint32_t bodyOffset, const std::vector<Material *>& materials, const std::vector<VertexBuffer *>& vertexBuffers) {
    static_assert(std::is_base_of<Mesh, T>::value, "T must be derived from Mesh");

    ResourceManager resourceManager = State::getResourceManager();

    struct LSWMeshHeader mesh_header;

    stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

    mesh_header.mesh_offset = stream.readUint32();

    if (!mesh_header.mesh_offset) {
      return;
    }

    uint32_t nextOffset = mesh_header.mesh_offset;
    unsigned i = 0;
    do {
      struct LSWProviders::LSWMesh lswMesh = LSWProviders::readMeshInfo(stream, bodyOffset, nextOffset);

      T *mesh = resourceManager.createResource<T>();
      meshes.push_back(mesh);

      Material *material = materials.at(lswMesh.materialIdx);
      mesh->setMaterial(material);

      ShaderType shaderType = LSWProviders::getShaderTypeFromMesh(lswMesh, material);
      mesh->setShaderType(shaderType);

      const VertexLayout& vertexLayout = LSWProviders::getVertexLayoutFromMesh(lswMesh);
      mesh->setVertexLayout(vertexLayout);

      VertexBuffer *vertexBuffer = vertexBuffers.at(lswMesh.vertexBlockIdx - 1);
      mesh->setVertexBuffer(vertexBuffer);

      processSurfaces(stream, bodyOffset, lswMesh.surfacesOffset, mesh);

      nextOffset = lswMesh.next_offset;
      i++;
    } while (nextOffset);
  }

  inline const struct LSWProviders::LSWMesh LSWProviders::readMeshInfo(Stream &stream, const uint32_t body_offset, uint32_t mesh_offset) {
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
}

#endif
