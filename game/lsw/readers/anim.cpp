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
#include <cstdio>
#include <stdexcept>

#include "../../../log.hpp"
#include "../../../state.hpp"
#include "anim.hpp"

using namespace Mortar::Game::LSW::Readers;

struct LSWAnimFileHeader {
  uint32_t version;
  uint32_t globalAdjust;
  uint32_t dataHeaderOffset;
};

struct LSWAnimDataHeader {
  float lengthInFrames;
  uint16_t elementCount;
  uint16_t channelsPerElementCount;
  uint16_t intervalCount;

  uint16_t unk_000a;

  uint32_t channelsOffset;
  uint32_t keyframeTypesOffset;
  uint32_t flagsOffset;
};

struct LSWAnimChannel {
  uint32_t keyframeMasksOffset;
  uint32_t intervalOffsetsOffset;
  uint32_t dataOffset;
};

inline Mortar::Resource::Animation::KeyframeType translateKeyframeType(uint8_t type) {
  switch (type) {
    case 0:
      return Mortar::Resource::Animation::KeyframeType::NONE;
    case 1:
      return Mortar::Resource::Animation::KeyframeType::FLOAT;
    case 4:
      return Mortar::Resource::Animation::KeyframeType::BOOLEAN;
    default:
      DEBUG("unsupported keyframe type %d", type);
      throw std::runtime_error("unsupported keyframe type");
  }
}

Mortar::Resource::Animation *AnimReader::read(Stream& stream) {
  Mortar::Resource::ResourceManager resourceManager = Mortar::State::getResourceManager();
  Mortar::Resource::Animation *animation = resourceManager.createResource<Mortar::Resource::Animation>();

  struct LSWAnimFileHeader fileHeader;
  fileHeader.version = stream.readUint32();
  fileHeader.globalAdjust = stream.readUint32();
  fileHeader.dataHeaderOffset = stream.readUint32();

  struct LSWAnimDataHeader dataHeader;
  stream.seek(fileHeader.dataHeaderOffset - fileHeader.globalAdjust, SEEK_SET);
  dataHeader.lengthInFrames = stream.readFloat();
  dataHeader.elementCount = stream.readUint16();
  dataHeader.channelsPerElementCount = stream.readUint16();
  dataHeader.intervalCount = stream.readUint16();

  stream.seek(sizeof(uint16_t), SEEK_CUR);

  dataHeader.channelsOffset = stream.readUint32();
  dataHeader.keyframeTypesOffset = stream.readUint32();
  dataHeader.flagsOffset = stream.readUint32();

  animation->setLength(dataHeader.lengthInFrames);
  animation->setIntervalCount(dataHeader.intervalCount);

  unsigned totalChannelCount = dataHeader.elementCount * dataHeader.channelsPerElementCount;

  std::vector<uint8_t> keyframeTypes (totalChannelCount);

  stream.seek(dataHeader.keyframeTypesOffset - fileHeader.globalAdjust, SEEK_SET);
  for (int i = 0; i < totalChannelCount; i++) {
    uint8_t type = stream.readUint8();
    keyframeTypes[i] = type;
  }

  std::vector<uint8_t> elementFlags (dataHeader.elementCount);

  stream.seek(dataHeader.flagsOffset - fileHeader.globalAdjust, SEEK_SET);
  for (int i = 0; i < dataHeader.elementCount; i++) {
    elementFlags[i] = stream.readUint8();
  }

  std::vector<uint32_t> channelOffsets (totalChannelCount);
  std::vector<float> noneTypeValues (totalChannelCount);

  stream.seek(dataHeader.channelsOffset - fileHeader.globalAdjust, SEEK_SET);
  for (int i = 0; i < totalChannelCount; i++) {
    if (translateKeyframeType(keyframeTypes[i]) == Resource::Animation::KeyframeType::NONE) {
      // NONE-type keyframes don't use the same structure all the other types
      // do; in place of the offset to that struct there's a single float
      noneTypeValues[i] = stream.readFloat();
      continue;
    }

    channelOffsets[i] = stream.readUint32();
  }

  std::vector<LSWAnimChannel> lswChannels (totalChannelCount);
  for (int i = 0; i < totalChannelCount; i++) {
    if (translateKeyframeType(keyframeTypes[i]) == Resource::Animation::KeyframeType::NONE) {
      continue;
    }

    stream.seek(channelOffsets[i] - fileHeader.globalAdjust, SEEK_SET);

    lswChannels[i].keyframeMasksOffset = stream.readUint32();
    lswChannels[i].intervalOffsetsOffset = stream.readUint32();
    lswChannels[i].dataOffset = stream.readUint32();

    assert(lswChannels[i].keyframeMasksOffset != 0);
    assert(lswChannels[i].intervalOffsetsOffset != 0);
    assert(lswChannels[i].dataOffset != 0);
  }

  for (int i = 0; i < dataHeader.elementCount; i++) {
    Mortar::Resource::Animation::Element *element = resourceManager.createResource<Mortar::Resource::Animation::Element>();
    animation->addElement(element);

    uint32_t flags = elementFlags.at(i);

    element->setHasRotation((flags & 1) != 0);
    element->setHasScale((flags & 8) != 0);
    element->setIsRelativeToJoint((flags & 0x20) != 0);

    for (int j = 0; j < dataHeader.channelsPerElementCount; j++) {
      unsigned channelIdx = i * dataHeader.channelsPerElementCount + j;

      Mortar::Resource::Animation::Channel *channel = resourceManager.createResource<Mortar::Resource::Animation::Channel>();
      element->addChannel(channel);

      Mortar::Resource::Animation::KeyframeType keyframeType = translateKeyframeType(keyframeTypes[channelIdx]);
      channel->setKeyframeType(keyframeType);

      if (keyframeType == Mortar::Resource::Animation::KeyframeType::NONE) {
        channel->setData(noneTypeValues[channelIdx]);

        continue;
      }

      LSWAnimChannel& lswChannel = lswChannels.at(channelIdx);
      unsigned keyframeCount = 0;

      stream.seek(lswChannel.keyframeMasksOffset - fileHeader.globalAdjust, SEEK_SET);
      for (int k = 0; k < dataHeader.intervalCount; k++) {
        uint8_t keyframeMask[4];

        keyframeMask[0] = stream.readUint8();
        keyframeMask[1] = stream.readUint8();
        keyframeMask[2] = stream.readUint8();
        keyframeMask[3] = stream.readUint8();

        channel->addKeyframeMask(keyframeMask[0], keyframeMask[1], keyframeMask[2], keyframeMask[3]);

        keyframeCount += std::bitset<8>(keyframeMask[0]).count();
        keyframeCount += std::bitset<8>(keyframeMask[1]).count();
        keyframeCount += std::bitset<8>(keyframeMask[2]).count();
        keyframeCount += std::bitset<8>(keyframeMask[3]).count();
      }

      stream.seek(lswChannel.intervalOffsetsOffset - fileHeader.globalAdjust, SEEK_SET);
      for (int k = 0; k < dataHeader.intervalCount; k++) {
        uint16_t offset = stream.readUint16();
        channel->addIntervalOffset(offset);
      }

      stream.seek(lswChannel.dataOffset - fileHeader.globalAdjust, SEEK_SET);
      if (keyframeType == Mortar::Resource::Animation::KeyframeType::FLOAT) {
        unsigned floatCount = (keyframeCount + 1) * 4;

        float *data = (float *)calloc(floatCount, sizeof(float));
        for (int k = 0; k < floatCount; k++) {
          data[k] = stream.readFloat();
        }

        channel->setData(data, floatCount * sizeof(float));
      } else {
        DEBUG("unimplemented keyframe type %d", keyframeType);
        throw std::runtime_error("unimplemented keyframe type");
      }
    }
  }

  return animation;
}
