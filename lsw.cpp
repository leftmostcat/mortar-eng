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

#include "log.hpp"
#include "lsw.hpp"
#include "shader.hpp"

using namespace Mortar::LSW;

static struct Mesh readMeshInfo(Stream &stream, const uint32_t body_offset, uint32_t mesh_offset) {
  stream.seek(body_offset + mesh_offset, SEEK_SET);
  struct Mesh mesh;

  memset(&mesh, 0, sizeof(struct Mesh));

  mesh.next_offset = stream.readUint32();

  stream.seek(1 * sizeof(uint32_t), SEEK_CUR);

  mesh.material_idx = stream.readUint32();
  mesh.vertex_type = stream.readUint32();

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  mesh.vertex_block_idx = stream.readUint32();

  stream.seek(1 * sizeof(uint32_t), SEEK_CUR);

  mesh.unk_0024 = stream.readUint32();

  stream.seek(2 * sizeof(uint32_t), SEEK_CUR);

  mesh.face_offset = stream.readUint32();

  stream.seek(4, SEEK_CUR);

  mesh.unk_0038 = stream.readUint32();
  mesh.unk_003C = stream.readUint32();

  return mesh;
}

static struct Face readFaceInfo(Stream &stream, const uint32_t body_offset, uint32_t face_offset) {
  stream.seek(body_offset + face_offset, SEEK_SET);
  struct Face face;

  memset(&face, 0, sizeof(struct Face));

  face.next_offset = stream.readUint32();
  face.primitive_type = stream.readUint32();

  face.num_elements = stream.readUint16();

  stream.seek(sizeof(uint16_t), SEEK_CUR);

  face.elements_offset = stream.readUint32();

  return face;
}

std::vector<Model::Mesh> Mortar::LSW::processMeshHeader(Stream &stream, const uint32_t body_offset, uint32_t mesh_header_offset, std::vector<Model::VertexBuffer> &vertexBuffers) {
  std::vector<Model::Mesh> meshes;

  if (!mesh_header_offset) {
    return meshes;
  }

  stream.seek(body_offset + mesh_header_offset, SEEK_SET);
  struct MeshHeader mesh_header;

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  mesh_header.mesh_offset = stream.readUint32();

  if (!mesh_header.mesh_offset) {
    return meshes;
  }

  struct Mesh mesh_data = readMeshInfo(stream, body_offset, mesh_header.mesh_offset);
  bool processNextMesh;

  do {
    Model::Mesh mesh;

    int stride = 0;

    /* Vertex stride is specified per-mesh. */
    switch (mesh_data.vertex_type) {
      case 0x59:
        stride = 36;
        break;
      case 0x5d:
        stride = 56;
        break;
      default:
        fprintf(stderr, "Unknown vertex type %d\n", mesh_data.vertex_type);
    }

    mesh.vertex_buffer_idx = mesh_data.vertex_block_idx - 1;
    mesh.material_idx = mesh_data.material_idx;
    mesh.rawVertexType = mesh_data.vertex_type;

    struct Face face_data = readFaceInfo(stream, body_offset, mesh_data.face_offset);
    bool processNextFace;

    if (mesh_data.vertex_type == 0x5d || mesh_data.unk_0038 != 0) {
      mesh.skinned = true;
    }
    if (mesh_data.unk_0024 != 0 && mesh_data.unk_003C != 0) {
      mesh.blended = true;
    }

    do {
      Model::Face face;

      face.primitive_type = face_data.primitive_type;
      face.materialIdx = mesh.material_idx;
      face.stride = stride;
      face.num_elements = face_data.num_elements;

      face.element_buffer = new uint16_t[face_data.num_elements];
      face.vertex_buffer_idx = mesh.vertex_buffer_idx;

      stream.seek(body_offset + face_data.elements_offset, SEEK_SET);

      for (int i = 0; i < face_data.num_elements; i++) {
        face.element_buffer[i] = stream.readUint16();
      }

      processNextFace = false;
      mesh.faces.push_back(face);

      if (face_data.next_offset) {
        face_data = readFaceInfo(stream, body_offset, face_data.next_offset);
        processNextFace = true;
      }
    } while (processNextFace);

    processNextMesh = false;
    meshes.push_back(mesh);

    if (mesh_data.next_offset) {
      mesh_data = readMeshInfo(stream, body_offset, mesh_data.next_offset);
      processNextMesh = true;
    }
  } while (processNextMesh);

  return meshes;
}
