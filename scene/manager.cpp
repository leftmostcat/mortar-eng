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

#include <vector>

#include "../log.hpp"
#include "../state.hpp"
#include "../resource/actor.hpp"
#include "manager.hpp"

using namespace Mortar::Scene;

void SceneManager::initialize(Render::Renderer *renderer) {
  this->renderer = renderer;

  renderer->initialize();
}

void SceneManager::shutDown() {
  renderer->shutDown();
}

Mortar::Resource::Actor *SceneManager::addActor(Mortar::Resource::Character::Character *character, glm::mat4 worldTransform) {
  // XXX: Make it so there can be more than one actor, duh
  Mortar::Resource::Actor *actor = State::getResourceManager().getResource<Mortar::Resource::Actor>("foo");
  this->actors.push_back(actor);

  actor->setCharacter(character);
  actor->setWorldTransform(worldTransform);

  this->renderer->registerTextures(character->getModel()->getTextures());
  this->renderer->registerVertexBuffers(character->getModel()->getVertexBuffers());

  const std::vector<Resource::Layer *>& layers = character->getLayers();
  for (auto layer = layers.begin(); layer != layers.end(); layer++) {
    const std::vector<Resource::DeformableSkinMesh *>& deformableSkinMeshes = (*layer)->getDeformableSkinMeshes();
    const std::vector<Resource::KinematicMesh *>& kinematicMeshes = (*layer)->getKinematicMeshes();
    const std::vector<Resource::SkinMesh *>& skinMeshes = (*layer)->getSkinMeshes();

    std::vector<Resource::Mesh *> meshes;
    meshes.reserve(deformableSkinMeshes.size() + kinematicMeshes.size() + skinMeshes.size());

    meshes.insert(meshes.end(), deformableSkinMeshes.begin(), deformableSkinMeshes.end());
    meshes.insert(meshes.end(), kinematicMeshes.begin(), kinematicMeshes.end());
    meshes.insert(meshes.end(), skinMeshes.begin(), skinMeshes.end());

    this->renderer->registerMeshes(meshes);
  }

  return actor;
}

class GeometryList {
  public:
    void addGeom(Mortar::Resource::GeomObject *geom);

    Mortar::Resource::GeomObject *head;
    Mortar::Resource::GeomObject *tail;
};

void GeometryList::addGeom(Mortar::Resource::GeomObject *geom) {
  if (head == nullptr) {
    head = geom;
  } else {
    tail->setNext(geom);
  }

  tail = geom;
}

void SceneManager::render() {
  Resource::ResourceManager resourceManager = State::getResourceManager();

  struct GeometryList geoms { nullptr, nullptr };
  struct GeometryList alphaGeoms { nullptr, nullptr };

  // XXX: use character config to determine enabled layers
  std::vector<unsigned> enabledLayers { 0, 2 };

  for (auto actor = this->actors.begin(); actor != this->actors.end(); actor++) {
    const Resource::Character::Character *character = (*actor)->getCharacter();

    for (auto enabledLayer = enabledLayers.begin(); enabledLayer != enabledLayers.end(); enabledLayer++) {
      const Resource::Layer *layer = character->getLayer(*enabledLayer);

      const std::vector<Resource::DeformableSkinMesh *>& deformableSkinMeshes = layer->getDeformableSkinMeshes();
      for (auto mesh = deformableSkinMeshes.begin(); mesh != deformableSkinMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->clear();

        geom->setMesh(*mesh);

        glm::mat4 transform = (*actor)->getWorldTransform() * character->getSkinTransform(0) * character->getJoint(0)->getRestPoseTransform();
        geom->setWorldTransform(transform);

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.addGeom(geom);
        } else {
          geoms.addGeom(geom);
        }
      }

      const std::vector<Resource::SkinMesh *>& skinMeshes = layer->getSkinMeshes();
      for (auto mesh = skinMeshes.begin(); mesh != skinMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->clear();

        geom->setMesh(*mesh);

        glm::mat4 transform = (*actor)->getWorldTransform() * character->getSkinTransform(0) * character->getJoint(0)->getRestPoseTransform();
        geom->setWorldTransform(transform);

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.addGeom(geom);
        } else {
          geoms.addGeom(geom);
        }
      }

      const std::vector<Resource::Joint *> joints = character->getJoints();

      std::vector<glm::mat4> boneTransforms (joints.size());
      for (int i = 0; i < joints.size(); i++) {
        const Resource::Joint *joint = joints.at(i);
        const size_t parentIdx = joint->getParentIdx();
        if (parentIdx != -1) {
          boneTransforms[i] = joint->getRestPoseTransform() * boneTransforms[parentIdx];
        } else {
          boneTransforms[i] = joint->getRestPoseTransform();
        }
      }

      const std::vector<Resource::KinematicMesh *>& kinematicMeshes = layer->getKinematicMeshes();
        for (auto mesh = kinematicMeshes.begin(); mesh != kinematicMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->clear();

        geom->setMesh(*mesh);

        glm::mat4 transform = (*actor)->getWorldTransform() * boneTransforms.at((*mesh)->getJointIdx());
        geom->setWorldTransform(transform);

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.addGeom(geom);
        } else {
          geoms.addGeom(geom);
        }
      }
    }
  }

  if (geoms.tail) {
    geoms.tail->setNext(alphaGeoms.head);
  } else {
    geoms.head = alphaGeoms.head;
  }

  this->renderer->renderGeometry(geoms.head);

  resourceManager.clearGeomObjectPool();
}
