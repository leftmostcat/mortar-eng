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

int Joint::getParentIdx() const {
  return this->parentIdx;
}

void Joint::setParentIdx(int parentIdx) {
  this->parentIdx = parentIdx;
}

const Mortar::Math::Matrix& Joint::getTransform() const {
  return this->transform;
}

void Joint::setTransform(Mortar::Math::Matrix &transform) {
  this->transform = transform;
}

const Mortar::Math::Point& Joint::getAttachmentPoint() const {
  return this->attachmentPoint;
}

void Joint::setAttachmentPoint(Mortar::Math::Point &attachmentPoint) {
  this->attachmentPoint = attachmentPoint;
}

void Joint::setFlag(Joint::Flags flag, bool value) {
  if (value) {
    this->flags |= flag;
  } else {
    this->flags &= ~flag;
  }
}

bool Joint::getIsRelativeToAttachment() const {
  return this->flags & IS_RELATIVE_TO_ATTACHMENT;
}

void Joint::setIsRelativeToAttachment(bool isRelativeToAttachment) {
  this->setFlag(IS_RELATIVE_TO_ATTACHMENT, isRelativeToAttachment);
}
