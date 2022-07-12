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
#include <bitset>
#include <stdexcept>
#include <stdint.h>

#include "../log.hpp"
#include "../state.hpp"
#include "../math/matrix.hpp"
#include "anim.hpp"

using namespace Mortar::Animation;

struct KeyframeLookupKey {
  // Floating point frame position
  float position;

  // Animations are divided into intervals of 32 frames
  unsigned interval;

  // Integer position of current frame after current interval
  unsigned char subinterval;

  uint8_t intervalMask;
};

float inline squared(float f) {
  return f * f;
}

float inline cubed(float f) {
  return f * f * f;
}

float inline calculateHermite(float position, float tStart, float tEnd, float inverseDuration, float blendStart, float blendEnd, float rateStart, float rateEnd) {
  float u = (position - tStart) * inverseDuration;
  float duration = tEnd - tStart;

  float durationAdjustedStartRate = duration * rateStart;
  float durationAdjustedEndRate = duration * rateEnd;

  return (2 * cubed(u) - 3 * squared(u) + 1) * blendStart + (-2 * cubed(u) + 3 * squared(u)) * blendEnd + (cubed(u) - 2 * squared(u) + u) * durationAdjustedStartRate + (cubed(u) - squared(u)) * durationAdjustedEndRate;
}

float calculateChannelValue(const Mortar::Resource::Animation::Channel *channel, const struct KeyframeLookupKey *key) {
  Mortar::Resource::Animation::KeyframeType keyframeType = channel->getKeyframeType();

  if (keyframeType == Mortar::Resource::Animation::KeyframeType::NONE) {
    return channel->getFloatData();
  } else if (keyframeType == Mortar::Resource::Animation::KeyframeType::BOOLEAN) {
    throw std::runtime_error("boolean keyframes unimplemented");
  }

  const uint8_t *mask = channel->getKeyframeMask(key->interval);

  unsigned keyframe = 0;
  for (int i = 0; i < key->subinterval; i++) {
    // Relies on subinterval to be zero-indexed so we can mask against the
    // current subinterval after the loop
    keyframe += std::bitset<8>(mask[i]).count();
  }
  keyframe += std::bitset<8>(mask[key->subinterval] & key->intervalMask).count();

  if (Mortar::State::printNextFrame) {
    DEBUG("mask 0x%x, 0x%x, 0x%x, 0x%x; interval mask 0x%x", mask[0], mask[1], mask[2], mask[3], key->intervalMask);
  }

  unsigned intervalOffset = channel->getIntervalOffset(key->interval);

  if (Mortar::State::printNextFrame) {
    DEBUG("keyframe %d, interval offset %d", keyframe, intervalOffset);
  }

  if (channel->getKeyframeType() == Mortar::Resource::Animation::KeyframeType::FLOAT) {
    unsigned offset = (intervalOffset + keyframe - 1) * 4;
    float *startData = (float *)channel->getData() + offset;
    float *endData = startData + 4;

    if (Mortar::State::printNextFrame) {
      DEBUG("t start %f, t end %f, invDur %f, p %f, q %f, v %f, w %f", startData[0], endData[0], startData[1], startData[2], endData[2], startData[3], endData[3]);
    }

    switch (Mortar::State::interpolate) {
      case Mortar::State::InterpolateType::NONE:
        return startData[2];
      case Mortar::State::InterpolateType::HERMITE:
        return calculateHermite(key->position, startData[0], endData[0], startData[1], startData[2], endData[2], startData[3], endData[3]);
    }
  } else {
    DEBUG("keyframe type %d", keyframeType);
    throw std::runtime_error("unimplemented keyframe type");
  }
}

struct KeyframeLookupKey createKeyframeLookup(unsigned intervalCount, float position) {
  struct KeyframeLookupKey key;

  key.position = position;
  if (key.position < 1.0f) {
    key.position = 1.0f;
  }

  key.interval = ((unsigned)key.position - 1) >> 5;
  if (key.interval >= intervalCount) {
    key.interval = intervalCount - 1;
  }

  float positionAfterInterval = key.position - (float)(key.interval << 5);
  unsigned intPositionAfterInterval = (unsigned)(positionAfterInterval - 1.0f);

  key.subinterval = (unsigned char)intPositionAfterInterval >> 3;

  if (key.subinterval < 0 || key.subinterval >= 4 || Mortar::State::printNextFrame) {
    DEBUG("position %f, interval %u, position after interval %f, subinterval %d", key.position, key.interval, positionAfterInterval, key.subinterval);
  }
  assert(key.subinterval >= 0 && key.subinterval < 4);

  key.intervalMask = (1 << (((intPositionAfterInterval & 0x7) + 1) & 0x1f)) - 1;

  return key;
}

std::vector<Mortar::Math::Matrix> Mortar::Animation::runSkeletalAnimation(const Mortar::Resource::Animation *animation, const std::vector<Mortar::Resource::Joint *>& joints, float position) {
  struct KeyframeLookupKey key = createKeyframeLookup(animation->getIntervalCount(), position);

  std::vector<Mortar::Math::Matrix> transforms;
  transforms.reserve(joints.size());

  for (int i = 0; i < joints.size(); i++) {
    if (i >= animation->getElementCount()) {
      transforms[i] = Math::Matrix();
      continue;
    }

    const Mortar::Resource::Animation::Element *element = animation->getElement(i);
    const Mortar::Resource::Joint *joint = joints.at(i);

    if (element->getHasRotation()) {
      float yaw = calculateChannelValue(element->getChannel(3), &key);
      float pitch = calculateChannelValue(element->getChannel(4), &key);
      float roll = calculateChannelValue(element->getChannel(5), &key);

      transforms.push_back(Mortar::Math::Matrix::rotationZYX(pitch, yaw, roll));
      transforms[i].transpose();
    } else {
      transforms.push_back(Mortar::Math::Matrix());
    }

    if (element->getIsRelativeToJoint()) {
      transforms[i] = transforms[i] * joint->getTransform();
    }

    if (element->getHasScale()) {
      float scaleX = calculateChannelValue(element->getChannel(6), &key);
      float scaleY = calculateChannelValue(element->getChannel(7), &key);
      float scaleZ = calculateChannelValue(element->getChannel(8), &key);

      transforms[i].scale(scaleX, scaleY, scaleZ);
    }

    float transX = calculateChannelValue(element->getChannel(0), &key);
    float transY = calculateChannelValue(element->getChannel(1), &key);
    float transZ = calculateChannelValue(element->getChannel(2), &key);

    transforms[i].translate(transX, transY, transZ);

    if (joint->getIsRelativeToAttachment()) {
      Mortar::Math::Point attachmentPoint = joint->getAttachmentPoint();

      Mortar::Math::Point transformedAttachment = attachmentPoint * transforms[i];
      transforms[i].setTranslation(transformedAttachment);

      transforms[i].translate(-attachmentPoint);
    }

    transforms[i]._13 = -transforms[i]._13;
    transforms[i]._23 = -transforms[i]._23;
    transforms[i]._31 = -transforms[i]._31;
    transforms[i]._32 = -transforms[i]._32;
    transforms[i]._43 = -transforms[i]._43;
  }

  return transforms;
}
