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

#include <forward_list>
#include <vector>

#include "../../../log.hpp"
#include "../../../math/matrix.hpp"
#include "../../../resource/scene.hpp"
#include "dds.hpp"
#include "nup.hpp"
#include "common.hpp"

using namespace Mortar::Game::LSW::Readers;

struct NUPHeader {
  uint32_t unk_0000;

  uint32_t strings_offset;
  uint32_t texture_header_offset;
  uint32_t material_header_offset;

  uint32_t unk_0010;

  uint32_t vertex_header_offset;
  uint32_t model_header_offset;
  uint32_t instances_offset;

  uint32_t unk_0020[2];
};

struct NUPModelHeader {
  uint32_t unk_0000[3];

  uint32_t num_materials;
  uint32_t num_mesh_blocks;
  uint32_t mesh_header_list_offset;
  uint32_t num_instances;

  uint32_t unk_001C;

  uint32_t num_special_objects;

  uint32_t unk_0024;

  uint32_t num_splines;
  uint32_t splines_offset;

  uint32_t unk_0030[106];
};

struct NUPInstance {
  Mortar::Math::Matrix transformation;

  uint16_t mesh_idx;
  uint16_t unk_0042;

  uint32_t unk_0044;

  uint32_t matrix_offset;

  uint32_t unk_004C;
};

struct NUPSpline {
  uint16_t vertexCount;

  uint16_t unk_0002;

  uint32_t nameOffset;
  uint32_t verticesOffset;
};

const int BODY_OFFSET = 0x40;

void NUPReader::read(Mortar::Resource::Scene *scene, Stream &stream) {
  Resource::ResourceManager resourceManager = State::getResourceManager();
  Resource::Model *model = resourceManager.createResource<Resource::Model>();
  scene->setModel(model);

  /* Read in NUP header at the top of the file. */
  stream.seek(0, SEEK_SET);
  struct NUPHeader file_header;

  stream.seek(4, SEEK_CUR);

  file_header.strings_offset = stream.readUint32();
  file_header.texture_header_offset = stream.readUint32();
  file_header.material_header_offset = stream.readUint32();

  stream.seek(sizeof(uint32_t), SEEK_CUR);

  file_header.vertex_header_offset = stream.readUint32();
  file_header.model_header_offset = stream.readUint32();
  file_header.instances_offset = stream.readUint32();

  /* Read in additional model information. */
  stream.seek(BODY_OFFSET + file_header.model_header_offset, SEEK_SET);
  struct NUPModelHeader model_header;

  stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

  model_header.num_materials = stream.readUint32();
  model_header.num_mesh_blocks = stream.readUint32();
  model_header.mesh_header_list_offset = stream.readUint32();
  model_header.num_instances = stream.readUint32();

  stream.seek(sizeof(uint32_t), SEEK_CUR);

  model_header.num_special_objects = stream.readUint32();

  stream.seek(sizeof(uint32_t), SEEK_CUR);

  model_header.num_splines = stream.readUint32();
  model_header.splines_offset = stream.readUint32();

  /* Read texture block information. */
  stream.seek(BODY_OFFSET + file_header.texture_header_offset, SEEK_SET);
  std::vector<Resource::Texture *> textures;
  CommonReaders::TexturesReader::read(textures, stream, BODY_OFFSET + file_header.texture_header_offset + 12);
  for (auto texture = textures.begin(); texture != textures.end(); texture++) {
    model->addTexture(*texture);
  }

  /* Read materials. */
  stream.seek(BODY_OFFSET + file_header.material_header_offset, SEEK_SET);
  std::vector<Resource::Material *> materials;
  CommonReaders::MaterialsReader::read(materials, stream, BODY_OFFSET, textures);

  /* Read vertex data. */
  stream.seek(BODY_OFFSET + file_header.vertex_header_offset, SEEK_SET);
  std::vector<Resource::VertexBuffer *> vertexBuffers;
  CommonReaders::VertexBufferReader::read(vertexBuffers, stream, BODY_OFFSET + file_header.vertex_header_offset);
  for (auto vertexBuffer = vertexBuffers.begin(); vertexBuffer != vertexBuffers.end(); vertexBuffer++) {
    model->addVertexBuffer(*vertexBuffer);
  }

  /* Break the layers down into meshes and add those to the model's list. */
  stream.seek(BODY_OFFSET + model_header.mesh_header_list_offset, SEEK_SET);
  uint32_t *mesh_header_offsets = new uint32_t[model_header.num_mesh_blocks];

  for (int i = 0; i < model_header.num_mesh_blocks; i++) {
    mesh_header_offsets[i] = stream.readUint32();
  }

  std::vector<std::forward_list<Resource::Mesh *>> meshes;
  for (int i = 0; i < model_header.num_mesh_blocks; i++) {
    stream.seek(BODY_OFFSET + mesh_header_offsets[i], SEEK_SET);

    std::vector<Resource::Mesh *> blockMeshes;
    CommonReaders::MeshesReader::read<Resource::Mesh>(blockMeshes, stream, BODY_OFFSET, materials, vertexBuffers);

    std::forward_list<Resource::Mesh *> meshList;
    for (auto mesh = blockMeshes.begin(); mesh != blockMeshes.end(); mesh++) {
      scene->addMesh(*mesh);
      meshList.push_front(*mesh);
    }
    meshes.push_back(meshList);
  }

  delete [] mesh_header_offsets;

  stream.seek(BODY_OFFSET + file_header.instances_offset, SEEK_SET);
  NUPInstance *instances_data = new NUPInstance[model_header.num_instances];

  for (int i = 0; i < model_header.num_instances; i++) {
    instances_data[i].transformation = Math::Matrix::fromStream(stream);
    instances_data[i].mesh_idx = stream.readUint16();

    stream.seek(sizeof(uint16_t), SEEK_CUR);
    stream.seek(sizeof(uint32_t), SEEK_CUR);

    instances_data[i].matrix_offset = stream.readUint32();

    stream.seek(sizeof(uint32_t), SEEK_CUR);
  }

  for (int i = 0; i < model_header.num_instances; i++) {
    Resource::Instance *instance = resourceManager.createResource<Resource::Instance>();
    scene->addInstance(instance);

    if (instances_data[i].matrix_offset) {
      stream.seek(BODY_OFFSET + instances_data[i].matrix_offset, SEEK_SET);
      Math::Matrix transform = Math::Matrix::fromStream(stream);
      instance->setWorldTransform(transform);
    } else {
      instance->setWorldTransform(instances_data[i].transformation);
    }

    instance->setMeshes(meshes.at(instances_data[i].mesh_idx));
  }

  delete [] instances_data;

  std::vector<NUPSpline> nupSplines (model_header.num_splines);

  stream.seek(BODY_OFFSET + model_header.splines_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_splines; i++) {
    struct NUPSpline& spline = nupSplines[i];

    spline.vertexCount = stream.readUint16();

    stream.seek(sizeof(uint16_t), SEEK_CUR);

    spline.nameOffset = stream.readUint32();
    spline.verticesOffset = stream.readUint32();
  }

  for (int i = 0; i < model_header.num_splines; i++) {
    stream.seek(BODY_OFFSET + nupSplines[i].nameOffset, SEEK_SET);
    char *splineName = stream.readString();

    Resource::Spline *spline = resourceManager.createResource<Resource::Spline>();

    scene->addSpline(splineName, spline);

    stream.seek(BODY_OFFSET + nupSplines[i].verticesOffset, SEEK_SET);
    for (int j = 0; j < nupSplines[i].vertexCount; j++) {
      Math::Vector vertex = Math::Vector::fromStream(stream, 1.0f);
      spline->addVertex(vertex);
    }
  }
}
