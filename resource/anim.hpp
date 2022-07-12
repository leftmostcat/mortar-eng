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

#ifndef MORTAR_RESOURCE_ANIM_H
#define MORTAR_RESOURCE_ANIM_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "resource.hpp"

namespace Mortar::Resource {
  // This animation class is designed with LEGO Star Wars animation data in
  // mind; it's likely it will need to be altered significantly to accommodate
  // later games at some point
  class Animation : public Resource {
    public:
      enum class KeyframeType {
        NONE,
        BOOLEAN,
        FLOAT,
      };

      class Channel : public Resource {
        public:
          Channel(ResourceHandle handle)
            : Resource { handle },
              data { nullptr } {};

          KeyframeType getKeyframeType() const;
          void setKeyframeType(KeyframeType type);

          void addKeyframeMask(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
          const uint8_t *getKeyframeMask(unsigned interval) const;

          void addIntervalOffset(size_t offset);
          size_t getIntervalOffset(unsigned interval) const;

          const void *getData() const;
          float getFloatData() const;
          size_t getDataSize() const;
          void setData(float data);
          void setData(void *data, size_t size);

        private:
          struct KeyframeMask {
            uint8_t subintervalMasks[4];
          };

          enum class DataType {
            NONE,
            FLOAT,
            POINTER,
          };

          KeyframeType keyframeType;
          std::vector<KeyframeMask> keyframeMasks;
          std::vector<size_t> intervalOffsets;

          DataType dataType;
          void *data;
          float floatData;
          size_t dataSize;
      };

      // The meaning of this is dependent on the animation type; for example, if
      // this is a skeletal animation, an element corresponds to a joint
      class Element : public Resource {
        public:
          Element(ResourceHandle handle)
            : Resource { handle },
              flags { 0 } {};

          void addChannel(Channel *channel);
          const Channel *getChannel(unsigned i) const;
          unsigned getChannelCount() const;

          bool getHasRotation() const;
          void setHasRotation(bool hasRotation);

          bool getHasScale() const;
          void setHasScale(bool hasScale);

          bool getIsRelativeToJoint() const;
          void setIsRelativeToJoint(bool isRelativeToJoint);

        private:
          enum Flags {
            HAS_ROTATION = 1 << 0,
            HAS_SCALE = 1 << 1,
            IS_RELATIVE_TO_JOINT = 1 << 2,
          };

          void setFlag(Flags flag, bool value);

          std::vector<Channel *> channels;
          unsigned char flags;
      };

      Animation(ResourceHandle handle)
        : Resource { handle } {};

      float getLength() const;
      void setLength(float length);

      unsigned getIntervalCount() const;
      void setIntervalCount(unsigned count);

      void addElement(Element *element);
      const Element *getElement(unsigned i) const;
      unsigned getElementCount() const;

    private:
      float length;
      unsigned intervalCount;
      std::vector<Element *> elements;
  };
}

#endif
