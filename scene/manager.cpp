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

#include <cmath>
#include <forward_list>
#include <list>
#include <stdexcept>
#include <vector>

#include "../anim/anim.hpp"
#include "../log.hpp"
#include "../state.hpp"
#include "../resource/actor.hpp"
#include "../resource/character.hpp"
#include "manager.hpp"

using namespace Mortar::Scene;

void SceneManager::initialize(Render::Renderer *renderer) {
  this->renderer = renderer;

  renderer->initialize();
}

void SceneManager::shutDown() {
  renderer->shutDown();
}

struct SockSegment {
  Mortar::Math::Vector minima { INFINITY, INFINITY, INFINITY, 1.0f };
  Mortar::Math::Vector maxima { -INFINITY, -INFINITY, -INFINITY, 1.0f };
};

void calculateSplineExtrema(const Mortar::Math::Vector& vertex, Mortar::Math::Vector& minima, Mortar::Math::Vector& maxima) {
  minima.x = fmin(minima.x, vertex.x);
  minima.y = fmin(minima.y, vertex.y);
  minima.z = fmin(minima.z, vertex.z);

  maxima.x = fmax(maxima.x, vertex.x);
  maxima.y = fmax(maxima.y, vertex.y);
  maxima.z = fmax(maxima.z, vertex.z);
}

float getValue(const Mortar::Math::Vector& lookAt, const Mortar::Math::Vector& a, const Mortar::Math::Vector& b) {
  // XXX: Currently do not understand the geometric significance of this blend calculation
  float angle;
  if (a.z - b.z == 0.0f) {
    angle = 0.0f;
  }
  angle = atan((a.x - b.x) / (a.z - b.z));

  return fabs((lookAt.x - b.x) * cos(-angle) + (lookAt.z - b.z) * sin(-angle));
}

Mortar::Resource::Actor *SceneManager::addActor(const Resource::Character *character, Math::Matrix worldTransform) {
  static unsigned actorCount = 0;

  Mortar::Resource::Actor *actor = State::getResourceManager().createResource<Mortar::Resource::Actor>();
  this->actors.push_back(actor);

  actor->setCharacter(character);
  actor->setWorldTransform(worldTransform);
  actor->setAnimation(Resource::Character::AnimationType::IDLE);

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

  actorCount++;

  return actor;
}

void SceneManager::setScene(const Resource::Scene *scene) {
  this->scene = scene;

  this->renderer->registerTextures(scene->getModel()->getTextures());
  this->renderer->registerVertexBuffers(scene->getModel()->getVertexBuffers());
  this->renderer->registerMeshes(scene->getMeshes());

  Math::Vector player1Pos;

  std::vector<Math::Matrix> pcStartingTransforms;

  const Resource::Spline *startSpline = scene->getSplineByName("start");
  if (startSpline != nullptr) {
    for (int i = 0; i < startSpline->getVertexCount(); i += 2) {
      Math::Vector position = startSpline->getVertex(i);
      Math::Vector lookAt = startSpline->getVertex(i + 1);

      float angleFromZ = lookAt.getAngleFrom(-Math::Vector::zAxis);

      Math::Matrix transform = Math::Matrix::rotationY(angleFromZ);
      transform.setTranslation(position);

      pcStartingTransforms.push_back(transform);

      if (i == 0) {
        player1Pos = position;
      }
    }
  }

  Math::Vector camPos;

  for (int i = 0; i < 0x20; i++) {
    char nameBuf[32];
    sprintf(nameBuf, "sock_cam_%.2d", i);

    const Resource::Spline *camSpline = scene->getSplineByName(nameBuf);
    if (camSpline != nullptr) {
      size_t vertexCount = camSpline->getVertexCount();

      sprintf(nameBuf, "sock_a_%.2d", i);
      const Resource::Spline *aSpline = scene->getSplineByName(nameBuf);

      sprintf(nameBuf, "sock_b_%.2d", i);
      const Resource::Spline *bSpline = scene->getSplineByName(nameBuf);

      if (aSpline == nullptr || bSpline == nullptr || aSpline->getVertexCount() != vertexCount && bSpline->getVertexCount() != vertexCount) {
        continue;
      }

      Math::Vector minima { INFINITY, INFINITY, INFINITY, 1.0f };
      Math::Vector maxima { -INFINITY, -INFINITY, -INFINITY, 1.0f };

      std::vector<SockSegment> segments (vertexCount - 1);

      // XXX: Factor in C and D splines
      for (int j = 0; j < vertexCount; j++) {
        Math::Vector aVertex = aSpline->getVertex(j);
        Math::Vector bVertex = bSpline->getVertex(j);

        calculateSplineExtrema(aVertex, minima, maxima);
        calculateSplineExtrema(bVertex, minima, maxima);

        if (j > 0) {
          calculateSplineExtrema(aVertex, segments[j - 1].minima, segments[j - 1].maxima);
          calculateSplineExtrema(bVertex, segments[j - 1].minima, segments[j - 1].maxima);
        }

        if (j < segments.size()) {
          calculateSplineExtrema(aVertex, segments[j].minima, segments[j].maxima);
          calculateSplineExtrema(bVertex, segments[j].minima, segments[j].maxima);
        }
      }

      // XXX: Create a sock system to be part of the scene manager and maintain
      // these so that the camera can run this calculation per-frame
      if (player1Pos.x >= minima.x && player1Pos.y >= minima.y && player1Pos.z >= minima.z && player1Pos.x <= maxima.x && player1Pos.y <= maxima.y && player1Pos.z <= maxima.z) {
        DEBUG("player 1 in sock %d, %lu segments", i, vertexCount - 1);
        for (int j = 0; j < segments.size(); j++) {
          SockSegment segment = segments[j];

          if (player1Pos.x >= segment.minima.x && player1Pos.y >= segment.minima.y && player1Pos.z >= segment.minima.z && player1Pos.x <= segment.maxima.x && player1Pos.y <= segment.maxima.y && player1Pos.z <= segment.maxima.z) {
            DEBUG("player 1 in segment %d", j);

            // XXX: Don't fully understand the calculation of the blend value here
            float start = getValue(player1Pos, aSpline->getVertex(j), bSpline->getVertex(j));
            float end = getValue(player1Pos, aSpline->getVertex(j + 1), bSpline->getVertex(j + 1));
            float blend = start / (start + end);

            const Math::Vector& camStartVertex = camSpline->getVertex(j);
            camPos = (camSpline->getVertex(j + 1) - camStartVertex) * blend + camStartVertex;

            // Use the mid spline to provide an upper limit on Y
            // XXX: Read Y-limiting flag from scene config
            sprintf(nameBuf, "sock_mid_%.2d", i);
            const Resource::Spline *midSpline = scene->getSplineByName(nameBuf);
            if (midSpline == nullptr || midSpline->getVertexCount() != vertexCount) {
              // XXX: Don't have a way to deal with this right now
              throw std::runtime_error("missing or invalid mid spline");
            }

            const Math::Vector& midStartVertex = midSpline->getVertex(j);
            Math::Vector midBlended = (midSpline->getVertex(j + 1) - midStartVertex) * blend + midStartVertex;

            camPos.y = fmax(camPos.y, fmin(1.2f, midBlended.y));
          }
        }
      }
    }
  }

  Math::Vector lookAt = player1Pos;

  // XXX: Pulled the height value out of a text file, need to read it in
  lookAt.y = 0.42f * 0.5f;

  State::getCamera().setLookAt(lookAt);
  State::getCamera().setPosition(camPos);

  DEBUG("camera position %s, look at %s", camPos.toString().c_str(), lookAt.toString().c_str());

  const std::vector<const Resource::Character *>& playerCharacters = scene->getPlayerCharacters();
  assert(playerCharacters.size() == pcStartingTransforms.size());

  for (int i = 0; i < playerCharacters.size(); i++) {
    this->addActor(playerCharacters.at(i), pcStartingTransforms.at(i));
  }
}

const std::vector<Mortar::Math::Matrix> calculatePose(const Mortar::Resource::Actor *actor) {
  const Mortar::Resource::Character *character = actor->getCharacter();

  if (actor->getAnimation() == Mortar::Resource::Character::Character::AnimationType::NONE) {
    return character->getRestPose();
  }

  const Mortar::Resource::Animation *anim = character->getSkeletalAnimation(actor->getAnimation());
  if (!anim) {
    throw std::runtime_error("character doesn't have that animation");
  }

  return Mortar::Animation::runSkeletalAnimation(anim, character->getJoints(), actor->getAnimationPosition());
}

void SceneManager::render() {
  Resource::ResourceManager resourceManager = State::getResourceManager();

  std::list<const Resource::GeomObject *> geoms;
  std::list<const Resource::GeomObject *> alphaGeoms;

  // XXX: use character config to determine enabled layers
  std::vector<unsigned> enabledLayers { 0, 2 };

  float timeDelta = State::getClock().getTimeDelta() * State::animRate;

  for (auto actor = this->actors.begin(); actor != this->actors.end(); actor++) {
    if (State::animEnabled && (*actor)->getAnimation() != Resource::Character::Character::AnimationType::IDLE) {
      (*actor)->setAnimation(Resource::Character::Character::AnimationType::IDLE);
    } else if (!State::animEnabled) {
      (*actor)->setAnimation(Resource::Character::Character::AnimationType::NONE);
    }

    const Resource::Character *character = (*actor)->getCharacter();

    (*actor)->advanceAnimation(timeDelta);

    const std::vector<Math::Matrix> pose = calculatePose(*actor);

    const std::vector<Resource::Joint *> joints = character->getJoints();

    std::vector<Math::Matrix> boneTransforms (joints.size());
    for (int i = 0; i < joints.size(); i++) {
      const Resource::Joint *joint = joints.at(i);
      const int parentIdx = joint->getParentIdx();

      if (State::printNextFrame) {
        Math::Matrix poseMtx = pose.at(i);
        DEBUG("transforming %d, parent %d\npose:\n%s", i, parentIdx, poseMtx.toString().c_str());
      }

      if (parentIdx != -1) {
        if (State::printNextFrame) {
          DEBUG("parent\n%s", boneTransforms[parentIdx].toString().c_str());
        }
        boneTransforms[i] = pose.at(i) * boneTransforms[parentIdx];
      } else {
        boneTransforms[i] = pose.at(i) * (*actor)->getWorldTransform();
      }

      if (State::printNextFrame) {
        DEBUG("result:\n%s", boneTransforms[i].toString().c_str());
      }
    }

    std::vector<Math::Matrix> skinTransforms (joints.size());
    for (int i = 0; i < joints.size(); i++) {
      skinTransforms[i] = character->getSkinTransform(i) * boneTransforms[i];
    }

    for (auto enabledLayer = enabledLayers.begin(); enabledLayer != enabledLayers.end(); enabledLayer++) {
      const Resource::Layer *layer = character->getLayer(*enabledLayer);

      const std::vector<Resource::DeformableSkinMesh *>& deformableSkinMeshes = layer->getDeformableSkinMeshes();
      for (auto mesh = deformableSkinMeshes.begin(); mesh != deformableSkinMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->reset();

        geom->setMesh(*mesh);
        geom->setSkinTransforms(skinTransforms);

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.push_back(geom);
        } else {
          geoms.push_back(geom);
        }
      }

      const std::vector<Resource::SkinMesh *>& skinMeshes = layer->getSkinMeshes();
      for (auto mesh = skinMeshes.begin(); mesh != skinMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->reset();

        geom->setMesh(*mesh);
        geom->setSkinTransforms(skinTransforms);

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.push_back(geom);
        } else {
          geoms.push_back(geom);
        }
      }

      const std::vector<Resource::KinematicMesh *>& kinematicMeshes = layer->getKinematicMeshes();
      for (auto mesh = kinematicMeshes.begin(); mesh != kinematicMeshes.end(); mesh++) {
        Resource::GeomObject *geom = resourceManager.getResource();
        geom->reset();

        geom->setMesh(*mesh);
        geom->setWorldTransform(boneTransforms.at((*mesh)->getJointIdx()));

        if ((*mesh)->getMaterial()->isAlphaBlended()) {
          alphaGeoms.push_back(geom);
        } else {
          geoms.push_back(geom);
        }
      }
    }
  }

  const std::vector<Resource::Instance *>& instances = this->scene->getInstances();
  for (auto instance = instances.begin(); instance != instances.end(); instance++) {
    std::forward_list<Resource::Mesh *> meshes = (*instance)->getMeshes();
    for (auto mesh = meshes.begin(); mesh != meshes.end(); mesh++) {
      Resource::GeomObject *geom = resourceManager.getResource();
      geom->reset();

      geom->setMesh(*mesh);
      geom->setWorldTransform((*instance)->getWorldTransform());

      if ((*mesh)->getMaterial()->isAlphaBlended()) {
        alphaGeoms.push_back(geom);
      } else {
        geoms.push_back(geom);
      }
    }
  }

  geoms.splice(geoms.end(), alphaGeoms);

  this->renderer->renderGeometry(geoms);

  resourceManager.clearGeomObjectPool();

  State::printNextFrame = false;
}
