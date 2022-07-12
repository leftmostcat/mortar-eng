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

#include <iterator>
#include <stdint.h>
#include <stdio.h>

#include "../../log.hpp"
#include "../../state.hpp"
#include "../../math/matrix.hpp"
#include "../../streams/filestream.hpp"
#include "../../streams/memorystream.hpp"
#include "../character/character.hpp"
#include "../joint.hpp"
#include "../layer.hpp"
#include "../mesh.hpp"
#include "../shader.hpp"
#include "dds.hpp"
#include "hgp.hpp"
#include "lsw/lsw.hpp"

using namespace Mortar::Resource::Providers;

struct HGPHeader {
  uint32_t unk_0000;

  uint32_t strings_offset;
  uint32_t texture_header_offset;
  uint32_t material_header_offset;

  uint32_t unk_0010;

  uint32_t vertex_header_offset;
  uint32_t model_header_offset;

  uint32_t unk_001C[2];

  uint16_t unk_0024[2];

  uint32_t unk_0028;

  uint32_t file_length;
};

struct HGPModelHeader {
  uint32_t unk_0000[5];

  uint32_t skeleton_offset;
  uint32_t rest_pose_offset;
  uint32_t skin_transforms_offset;
  uint32_t joint_index_map_offset;
  uint32_t layer_header_offset;
  uint32_t locators_offset;
  uint32_t locator_index_map_offset;
  uint32_t string_table_adjust;

  uint32_t unk_0034;
  float unk_0038;
  uint32_t unk_003C;
  float unk_0040;
  uint32_t unk_0044;
  float unk_0048[9];
  uint32_t unk_006C;
  uint32_t count_0070;
  uint32_t unk_0074;
  uint32_t unk_0078;

  uint8_t num_joints;
  uint8_t num_joint_indices;
  uint8_t num_layers;
  uint8_t num_locators;
  uint8_t num_locator_indices;

  uint8_t unk_0081[3];

  uint32_t unk_0084[11];
};

struct HGPLayerHeader {
  uint32_t name_offset;
  uint32_t mesh_header_list_offsets[4];
};

struct HGPJoint {
  Mortar::Math::Matrix transformation_mtx;
  Mortar::Math::Point attachment;

  uint32_t name_offset;

  int8_t parent_idx;
  uint8_t flags;

  uint8_t unk_0052[2];
  uint32_t unk_0054[3];
};

struct HGPLocator {
  Mortar::Math::Matrix transform;

  uint32_t unk_0040;

  uint8_t jointIdx;

  uint8_t unk_0045[11];
};

const uint32_t BODY_OFFSET = 0x30;

Mortar::Resource::Character::Character *HGPProvider::read(Character::CharacterDescription *description, const char *resourceName, Stream& stream) {
  ResourceManager resourceManager = State::getResourceManager();
  Character::Character *character = resourceManager.getResource<Character::Character>(resourceName);

  char *modelName = (char *)calloc(strlen(resourceName) + 7, sizeof(char));
  sprintf(modelName, "%s.model", resourceName);
  Model *model = resourceManager.getResource<Model>(modelName);
  character->setModel(model);

  /* Read in HGP header at the top of the file. */
  stream.seek(0, SEEK_SET);
  struct HGPHeader file_header;

  stream.seek(4, SEEK_CUR);

  file_header.strings_offset = stream.readUint32();
  file_header.texture_header_offset = stream.readUint32();
  file_header.material_header_offset = stream.readUint32();

  stream.seek(sizeof(uint32_t), SEEK_CUR);

  file_header.vertex_header_offset = stream.readUint32();
  file_header.model_header_offset = stream.readUint32();

  /* Read in additional model information. */
  stream.seek(BODY_OFFSET + file_header.model_header_offset, SEEK_SET);
  struct HGPModelHeader model_header;

  stream.seek(5 * sizeof(uint32_t), SEEK_CUR);

  model_header.skeleton_offset = stream.readUint32();
  model_header.rest_pose_offset = stream.readUint32();
  model_header.skin_transforms_offset = stream.readUint32();
  model_header.joint_index_map_offset = stream.readUint32();
  model_header.layer_header_offset = stream.readUint32();
  model_header.locators_offset = stream.readUint32();
  model_header.locator_index_map_offset = stream.readUint32();
  model_header.string_table_adjust = stream.readUint32();

  stream.seek(sizeof(uint32_t), SEEK_CUR);
  stream.seek(sizeof(float), SEEK_CUR);
  stream.seek(sizeof(uint32_t), SEEK_CUR);
  stream.seek(sizeof(float), SEEK_CUR);
  stream.seek(sizeof(uint32_t), SEEK_CUR);
  stream.seek(9 * sizeof(float), SEEK_CUR);
  stream.seek(4 * sizeof(uint32_t), SEEK_CUR);

  model_header.num_joints = stream.readUint8();
  model_header.num_joint_indices = stream.readUint8();
  model_header.num_layers = stream.readUint8();
  model_header.num_locators = stream.readUint8();
  model_header.num_locator_indices = stream.readUint8();

  /* Read texture block information. */
  stream.seek(BODY_OFFSET + file_header.texture_header_offset, SEEK_SET);
  std::vector<Texture *> textures;
  LSW::LSWProviders::TexturesProvider::read(textures, resourceName, stream, BODY_OFFSET + file_header.texture_header_offset + 12);
  for (auto texture = textures.begin(); texture != textures.end(); texture++) {
    model->addTexture(*texture);
  }

  /* Read materials. */
  stream.seek(BODY_OFFSET + file_header.material_header_offset, SEEK_SET);
  std::vector<Material *> materials;
  LSW::LSWProviders::MaterialsProvider::read(materials, resourceName, stream, BODY_OFFSET, textures);

  /* Read vertex data. */
  stream.seek(BODY_OFFSET + file_header.vertex_header_offset, SEEK_SET);
  std::vector<VertexBuffer *> vertexBuffers;
  LSW::LSWProviders::VertexBufferProvider::read(vertexBuffers, resourceName, stream, BODY_OFFSET + file_header.vertex_header_offset);
  for (auto vertexBuffer = vertexBuffers.begin(); vertexBuffer != vertexBuffers.end(); vertexBuffer++) {
    model->addVertexBuffer(*vertexBuffer);
  }

  /* Read skeleton. */
  std::vector<HGPJoint> hgpJoints (model_header.num_joints);

  stream.seek(BODY_OFFSET + model_header.skeleton_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_joints; i++) {
    struct HGPJoint& hgpJoint = hgpJoints.at(i);

    hgpJoint.transformation_mtx = Math::Matrix::fromStream(stream);
    hgpJoint.attachment = Math::Point::fromStream(stream);
    hgpJoint.name_offset = stream.readUint32();
    hgpJoint.parent_idx = stream.readInt8();
    hgpJoint.flags = stream.readUint8();

    stream.seek(2 * sizeof(uint8_t), SEEK_CUR);
    stream.seek(3 * sizeof(uint32_t), SEEK_CUR);
  }

  std::vector<Math::Matrix> restPose (model_header.num_joints);

  stream.seek(BODY_OFFSET + model_header.rest_pose_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_joints; i++) {
    restPose[i] = Math::Matrix::fromStream(stream);
  }

  character->setRestPose(restPose);

  for (int i = 0; i < model_header.num_joints; i++) {
    HGPJoint& hgpJoint = hgpJoints[i];

    stream.seek(BODY_OFFSET + file_header.strings_offset + file_header.strings_offset - model_header.string_table_adjust - model_header.skeleton_offset + hgpJoint.name_offset, SEEK_SET);
    char *jointName = stream.readString();

    char *jointResourceName = (char *)calloc(strlen(resourceName) + strlen(jointName) + 8, sizeof(char));
    sprintf(jointResourceName, "%s.joint.%s", resourceName, jointName);

    Joint *joint = resourceManager.getResource<Joint>(jointResourceName);
    character->addJoint(joint);

    joint->setName(jointName);

    joint->setParentIdx(hgpJoint.parent_idx);
    joint->setTransform(hgpJoint.transformation_mtx);

    joint->setAttachmentPoint(hgpJoint.attachment);

    joint->setIsRelativeToAttachment((hgpJoint.flags & 8) != 0);

    char parentMsg[256] = { 0 };
    if (joint->getParentIdx() != -1) {
      sprintf(parentMsg, ", parent is %s", character->getJoint(joint->getParentIdx())->getName());
    }
    DEBUG("joint %d is %s%s, flags 0x%x", i, jointName, parentMsg, hgpJoint.flags);
  }

  /* Read in information necessary for processing layers and meshes. */
  stream.seek(BODY_OFFSET + model_header.skin_transforms_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_joints; i++) {
    auto mtx = Math::Matrix::fromStream(stream);
    character->addSkinTransform(mtx);
  }

  stream.seek(BODY_OFFSET + model_header.layer_header_offset, SEEK_SET);
  std::vector<HGPLayerHeader> layer_headers (model_header.num_layers);

  for (int i = 0; i < model_header.num_layers; i++) {
    layer_headers[i].name_offset = stream.readUint32();

    for (int j = 0; j < 4; j++) {
      layer_headers[i].mesh_header_list_offsets[j] = stream.readUint32();
    }
  }

    /* Break the layers down into meshes and add those to the model's list. */
  for (int i = 0; i < model_header.num_layers; i++) {
    stream.seek(BODY_OFFSET + layer_headers[i].name_offset, SEEK_SET);
    char *layerName = stream.readString();

    char *layerResourceName = (char *)calloc(strlen(resourceName) + strlen(layerName) + 8, sizeof(char));
    sprintf(layerResourceName, "%s.layer.%s", resourceName, layerName);

    Layer *layer = resourceManager.getResource<Layer>(layerResourceName);
    character->addLayer(layer);

    for (int j = 0; j < 4; j++) {
      if (!layer_headers[i].mesh_header_list_offsets[j]) {
        continue;
      }

      stream.seek(BODY_OFFSET + layer_headers[i].mesh_header_list_offsets[j], SEEK_SET);

      if (j % 2 == 0) {
        std::vector<uint32_t> mesh_header_offsets (model_header.num_joints);

        for (unsigned k = 0; k < model_header.num_joints; k++) {
          mesh_header_offsets[k] = stream.readUint32();
        }

        for (unsigned k = 0; k < model_header.num_joints; k++) {
          if (!mesh_header_offsets[k]) {
            continue;
          }

          const char *jointName = character->getJoint(k)->getName();

          char *meshBlockName = (char *)calloc(strlen(layerResourceName) + strlen(jointName) + 8, sizeof(char));
          sprintf(meshBlockName, "%s.joint.%s", layerResourceName, jointName);

          stream.seek(BODY_OFFSET + mesh_header_offsets[k], SEEK_SET);
          std::vector<KinematicMesh *> kinematicMeshes;
          LSW::LSWProviders::MeshesProvider::read<KinematicMesh>(kinematicMeshes, meshBlockName, stream, BODY_OFFSET, materials, vertexBuffers);
          for (auto mesh = kinematicMeshes.begin(); mesh != kinematicMeshes.end(); mesh++) {
            (*mesh)->setJointIdx(k);
            layer->addKinematicMesh(*mesh);
          }
        }
      } else if (j == 1) {
        char *meshBlockName = (char *)calloc(strlen(layerResourceName) + 6, sizeof(char));
        sprintf(meshBlockName, "%s.skin", layerResourceName);

        std::vector<SkinMesh *> skinMeshes;
        LSW::LSWProviders::MeshesProvider::read<SkinMesh>(skinMeshes, meshBlockName, stream, BODY_OFFSET, materials, vertexBuffers);
        for (auto mesh = skinMeshes.begin(); mesh != skinMeshes.end(); mesh++) {
          layer->addSkinMesh(*mesh);
        }
      }
      else if (j == 3) {
        char *meshBlockName = (char *)calloc(strlen(layerResourceName) + 8, sizeof(char));
        sprintf(meshBlockName, "%s.deform", layerResourceName);

        std::vector<DeformableSkinMesh *> deformableSkinMeshes;
        LSW::LSWProviders::MeshesProvider::read<DeformableSkinMesh>(deformableSkinMeshes, meshBlockName, stream, BODY_OFFSET, materials, vertexBuffers);
        for (auto mesh = deformableSkinMeshes.begin(); mesh != deformableSkinMeshes.end(); mesh++) {
          layer->addDeformableSkinMesh(*mesh);
        }
      }
    }
  }

  stream.seek(BODY_OFFSET + model_header.locators_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_locators; i++) {
    HGPLocator hgpLocator;
    hgpLocator.transform = Math::Matrix::fromStream(stream);

    stream.seek(sizeof(uint32_t), SEEK_CUR);

    hgpLocator.jointIdx = stream.readUint8();

    stream.seek(11 * sizeof(uint8_t), SEEK_CUR);

    // XXX: Breaks if we have more than 99 locators
    char *locatorName = (char *)calloc(strlen(resourceName) + 7, sizeof(char));
    sprintf(locatorName, "%s.loc%.2d", resourceName, i);

    Character::Character::Locator *locator = resourceManager.getResource<Character::Character::Locator>(locatorName);
    character->addLocator(locator);

    locator->setTransform(hgpLocator.transform);
    locator->setJointIdx(hgpLocator.jointIdx);
  }

  stream.seek(BODY_OFFSET + model_header.locator_index_map_offset, SEEK_SET);
  for (int i = 0; i < model_header.num_locator_indices; i++) {
    uint8_t internal = stream.readUint8();
    character->addExternalLocatorMapping(i, internal);
  }

  return character;
}
