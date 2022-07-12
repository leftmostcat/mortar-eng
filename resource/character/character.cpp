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

#include <assert.h>
#include <vector>

#include "character.hpp"

using namespace Mortar::Resource::Character;

const Mortar::Resource::Model *Character::getModel() const {
  return this->model;
}

void Character::setModel(Mortar::Resource::Model *model) {
  this->model = model;
}

void Character::addJoint(Joint *joint) {
  this->joints.push_back(joint);
}

const Mortar::Resource::Joint *Character::getJoint(unsigned i) const {
  return this->joints.at(i);
}

const std::vector<Mortar::Resource::Joint *>& Character::getJoints() const {
  return this->joints;
}

const std::vector<Mortar::Math::Matrix>& Character::getRestPose() const {
  return this->restPose;
}

void Character::setRestPose(std::vector<Math::Matrix>& restPose) {
  this->restPose = restPose;
}

void Character::addSkinTransform(Math::Matrix& skinTransform) {
  this->skinTransforms.push_back(skinTransform);
}

const Mortar::Math::Matrix& Character::getSkinTransform(unsigned i) const {
  return this->skinTransforms.at(i);
}

void Character::addLayer(Layer *layer) {
  this->layers.push_back(layer);
}

const Mortar::Resource::Layer *Character::getLayer(unsigned i) const {
  return this->layers.at(i);
}

const std::vector<Mortar::Resource::Layer *>& Character::getLayers() const {
  return this->layers;
}

void Character::addLocator(Locator *locator) {
  this->locators.push_back(locator);
}

const Character::Locator *Character::getLocatorFromExternalIdx(unsigned char idx) const {
  return this->locators.at(this->externalLocatorMap.at(idx));
}

void Character::addExternalLocatorMapping(unsigned char external, unsigned char internal) {
  this->externalLocatorMap[external] = internal;
}

void Character::addSkeletalAnimation(AnimationType type, Animation *animation) {
  assert(type != AnimationType::NONE);
  this->skeletalAnimations[type] = animation;
}

bool Character::hasSkeletalAnimation(AnimationType type) const {
  return this->skeletalAnimations.contains(type);
}

const Mortar::Resource::Animation *Character::getSkeletalAnimation(AnimationType type) const {
  if (!this->hasSkeletalAnimation(type)) {
    return nullptr;
  }

  return this->skeletalAnimations.at(type);
}

const Mortar::Math::Matrix& Character::Locator::getTransform() const {
  return this->transform;
}

void Character::Locator::setTransform(const Math::Matrix &transform) {
  this->transform = transform;
}

unsigned char Character::Locator::getJointIdx() const {
  return this->jointIdx;
}

void Character::Locator::setJointIdx(unsigned char jointIdx) {
  this->jointIdx = jointIdx;
}
