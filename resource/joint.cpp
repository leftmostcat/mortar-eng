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

#include "joint.hpp"

using namespace Mortar::Resource;

const char *Joint::getName() const {
  return this->name;
}

void Joint::setName(const char *name) {
  this->name = name;
}

size_t Joint::getParentIdx() const {
  return this->parentIdx;
}

void Joint::setParentIdx(size_t parentIdx) {
  this->parentIdx = parentIdx;
}

const glm::mat4& Joint::getTransform() const {
  return this->transform;
}

void Joint::setTransform(glm::mat4 &transform) {
  this->transform = transform;
}

const glm::mat4& Joint::getRestPoseTransform() const {
  return this->restPoseTransform;
}

void Joint::setRestPoseTransform(glm::mat4 &transform) {
  this->restPoseTransform = transform;
}
