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

#ifndef MORTAR_RESOURCE_VERTEX_H
#define MORTAR_RESOURCE_VERTEX_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "../resource.hpp"

namespace Mortar::Resource {
  enum class VertexUsage {
    POSITION,
    NORMAL,
    COLOR,
    TEX_COORD,
    BLEND_WEIGHTS,
    BLEND_INDICES,
  };

  enum class VertexDataType {
    VEC3,
    VEC2,
    D3DCOLOR,
  };

  class VertexLayout {
    public:
      class VertexProperty {
        public:
          constexpr VertexProperty(VertexUsage usage, VertexDataType type, size_t offset)
            : usage { usage }, type { type }, offset { offset } {};

          size_t getOffset() const;
          VertexUsage getUsage() const;
          VertexDataType getDataType() const;

        private:
          VertexUsage usage;
          VertexDataType type;
          size_t offset;
      };

      constexpr VertexLayout(unsigned stride, std::vector<VertexProperty> properties)
        : stride { stride }, properties { properties } {};

      size_t getStride() const;
      const std::vector<VertexProperty>& getProperties() const;

      static VertexLayout EMPTY;

    private:
      unsigned stride;
      std::vector<VertexProperty> properties;
  };

  class IndexBuffer : public Resource {
    public:
      IndexBuffer(ResourceHandle handle)
        : Resource { handle } {};

      ~IndexBuffer();

      unsigned getCount() const;
      void setCount(unsigned count);

      const uint16_t *getData() const;
      void setData(uint16_t *data);

    private:
      unsigned count;
      uint16_t *data;
  };

  class VertexBuffer : public Resource {
    public:
      VertexBuffer(ResourceHandle handle)
        : Resource { handle } {};

      ~VertexBuffer();

      size_t getSize() const;
      void setSize(size_t size);

      const uint8_t *getData() const;
      void setData(uint8_t *data);

    private:
      size_t size;
      uint8_t *data;
  };
}

#endif
