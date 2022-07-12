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
#include <stdexcept>

#include "../log.hpp"
#include "anim.hpp"

using namespace Mortar::Resource;

float Animation::getLength() const {
  return this->length;
}

void Animation::setLength(float length) {
  this->length = length;
}

unsigned Animation::getIntervalCount() const {
  return this->intervalCount;
}

void Animation::setIntervalCount(unsigned int count) {
  this->intervalCount = count;
}

void Animation::addElement(Element *element) {
  this->elements.push_back(element);
}

Animation::KeyframeType Animation::Channel::getKeyframeType() const {
  return this->keyframeType;
}

void Animation::Channel::setKeyframeType(KeyframeType type) {
  this->keyframeType = type;

  if (type == KeyframeType::BOOLEAN) {
    this->dataType = DataType::NONE;
  }
}

void Animation::Channel::addKeyframeMask(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
  struct KeyframeMask mask;

  mask.subintervalMasks[0] = byte0;
  mask.subintervalMasks[1] = byte1;
  mask.subintervalMasks[2] = byte2;
  mask.subintervalMasks[3] = byte3;

  this->keyframeMasks.push_back(mask);
}

const uint8_t *Animation::Channel::getKeyframeMask(unsigned interval) const {
  return this->keyframeMasks.at(interval).subintervalMasks;
}

void Animation::Channel::addIntervalOffset(size_t offset) {
  this->intervalOffsets.push_back(offset);
}

size_t Animation::Channel::getIntervalOffset(unsigned interval) const {
  return this->intervalOffsets.at(interval);
}

const void *Animation::Channel::getData() const {
  assert(this->dataType == DataType::POINTER);

  return this->data;
}

float Animation::Channel::getFloatData() const {
  assert(this->dataType == DataType::FLOAT);

  return this->floatData;
}

size_t Animation::Channel::getDataSize() const {
  return this->dataSize;
}

void Animation::Channel::setData(float data) {
  assert(this->keyframeType == KeyframeType::NONE);

  this->dataType = DataType::FLOAT;
  this->floatData = data;
  this->dataSize = sizeof(float);
}

void Animation::Channel::setData(void *data, size_t size) {
  // NONE keyframes should use the float overload
  assert(this->keyframeType != KeyframeType::NONE);
  // BOOLEAN keyframes have no data associated with them
  assert(this->keyframeType != KeyframeType::BOOLEAN);

  this->dataType = DataType::POINTER;
  this->data = data;
  this->dataSize = size;
}

const Animation::Element *Animation::getElement(unsigned i) const {
  return this->elements.at(i);
}

unsigned Animation::getElementCount() const {
  return this->elements.size();
}

void Animation::Element::addChannel(Channel *channel) {
  this->channels.push_back(channel);
}

const Animation::Channel *Animation::Element::getChannel(unsigned i) const {
  return this->channels.at(i);
}

unsigned Animation::Element::getChannelCount() const {
  return this->channels.size();
}

void Animation::Element::setFlag(Animation::Element::Flags flag, bool value) {
  if (value) {
    this->flags |= flag;
  } else {
    this->flags &= ~flag;
  }
}

bool Animation::Element::getHasRotation() const {
  return this->flags & HAS_ROTATION;
}

void Animation::Element::setHasRotation(bool hasRotation) {
  this->setFlag(HAS_ROTATION, hasRotation);
}

bool Animation::Element::getHasScale() const {
  return this->flags & HAS_SCALE;
}

void Animation::Element::setHasScale(bool hasScale) {
  this->setFlag(HAS_SCALE, hasScale);
}

bool Animation::Element::getIsRelativeToJoint() const {
  return this->flags & IS_RELATIVE_TO_JOINT;
}

void Animation::Element::setIsRelativeToJoint(bool hasRotation) {
  this->setFlag(IS_RELATIVE_TO_JOINT, hasRotation);
}
