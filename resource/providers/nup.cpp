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

#include <stdio.h>

#include "../../log.hpp"
#include "../../state.hpp"
#include "../matrix.hpp"
#include "../scene.hpp"
#include "dds.hpp"
#include "nup.hpp"
#include "lsw/lsw.hpp"

using namespace Mortar::Resource::Providers;

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

  uint32_t unk_001C[111];
};

struct NUPInstance {
  glm::mat4 transformation;

  uint16_t mesh_idx;
  uint16_t unk_0042;

  uint32_t unk_0044;

  uint32_t matrix_offset;

  uint32_t unk_004C;
};

const int BODY_OFFSET = 0x40;

Mortar::Resource::Scene *NUPReader::read(const char *name, Stream &stream) {
  const char *path = State::getGameConfig()->getCharacterResourcePath(name);

  ResourceManager resourceManager = State::getResourceManager();
  Scene *scene = resourceManager.getResource<Scene>(path);

  char *modelName = (char *)calloc(strlen(path) + 7, sizeof(char));
  sprintf(modelName, "%s.model", path);
  Model *model = resourceManager.getResource<Model>(modelName);
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

  /* Read texture block information. */
  stream.seek(BODY_OFFSET + file_header.texture_header_offset, SEEK_SET);
  std::vector<Texture *> textures;
  LSW::LSWProviders::TexturesProvider::read(textures, path, stream, BODY_OFFSET + file_header.texture_header_offset + 12);
  for (auto texture = textures.begin(); texture != textures.end(); texture++) {
    model->addTexture(*texture);
  }

  /* Read materials. */
  stream.seek(BODY_OFFSET + file_header.material_header_offset, SEEK_SET);
  std::vector<Material *> materials;
  LSW::LSWProviders::MaterialsProvider::read(materials, path, stream, BODY_OFFSET, textures);

  /* Read vertex data. */
  stream.seek(BODY_OFFSET + file_header.vertex_header_offset, SEEK_SET);
  std::vector<VertexBuffer *> vertexBuffers;
  LSW::LSWProviders::VertexBufferProvider::read(vertexBuffers, path, stream, BODY_OFFSET + file_header.vertex_header_offset);
  for (auto vertexBuffer = vertexBuffers.begin(); vertexBuffer != vertexBuffers.end(); vertexBuffer++) {
    model->addVertexBuffer(*vertexBuffer);
  }

  /* Break the layers down into meshes and add those to the model's list. */
  stream.seek(BODY_OFFSET + model_header.mesh_header_list_offset, SEEK_SET);
  uint32_t *mesh_header_offsets = new uint32_t[model_header.num_mesh_blocks];

  for (int i = 0; i < model_header.num_mesh_blocks; i++) {
    mesh_header_offsets[i] = stream.readUint32();
  }

  std::vector<Mesh *> meshes;
  for (int i = 0; i < model_header.num_mesh_blocks; i++) {
    stream.seek(BODY_OFFSET + mesh_header_offsets[i], SEEK_SET);

    // XXX: Breaks if we have more than 99 mesh blocks
    char *meshBlockName = (char *)calloc(strlen(path) + 9, sizeof(char));
    sprintf(meshBlockName, "%s.block%.2d", path, i);

    std::vector<Mesh *> blockMeshes;
    LSW::LSWProviders::MeshesProvider::read<Mesh>(blockMeshes, meshBlockName, stream, BODY_OFFSET, materials, vertexBuffers);
    for (auto mesh = meshes.begin(); mesh != meshes.end(); mesh++) {
      scene->addMesh(*mesh);
      meshes.push_back(*mesh);
    }
  }

  delete [] mesh_header_offsets;

  stream.seek(BODY_OFFSET + file_header.instances_offset, SEEK_SET);
  NUPInstance *instances_data = new NUPInstance[model_header.num_instances];

  for (int i = 0; i < model_header.num_instances; i++) {
    instances_data[i].transformation = readMatrix(stream);
    instances_data[i].mesh_idx = stream.readUint16();

    stream.seek(sizeof(uint16_t), SEEK_CUR);
    stream.seek(sizeof(uint32_t), SEEK_CUR);

    instances_data[i].matrix_offset = stream.readUint32();

    stream.seek(sizeof(uint32_t), SEEK_CUR);
  }

  for (int i = 0; i < model_header.num_instances; i++) {
    // XXX: Breaks if we have more than 99 mesh blocks
    char *instanceName = (char *)calloc(strlen(path) + 8, sizeof(char));
    sprintf(instanceName, "%s.inst%.2d", path, i);

    Instance *instance = resourceManager.getResource<Instance>(instanceName);
    scene->addInstance(instance);

    if (instances_data[i].matrix_offset) {
      stream.seek(BODY_OFFSET + instances_data[i].matrix_offset, SEEK_SET);
      glm::mat4 transform = readMatrix(stream);
      instance->setWorldTransform(transform);
    }
    else {
      instance->setWorldTransform(instances_data[i].transformation);
    }

    instance->setMesh(meshes.at(instances_data[i].mesh_idx));
  }

  delete [] instances_data;

  return scene;
}
