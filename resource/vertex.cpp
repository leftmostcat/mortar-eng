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

#include "vertex.hpp"

using namespace Mortar::Resource;

VertexLayout VertexLayout::EMPTY = VertexLayout(0, {});

const std::vector<VertexLayout::VertexProperty>& VertexLayout::getProperties() const {
  return this->properties;
}

size_t VertexLayout::getStride() const {
  return this->stride;
}

size_t VertexLayout::VertexProperty::getOffset() const {
  return this->offset;
}

VertexUsage VertexLayout::VertexProperty::getUsage() const {
  return this->usage;
}

VertexDataType VertexLayout::VertexProperty::getDataType() const {
  return this->type;
}

unsigned IndexBuffer::getCount() const {
  return this->count;
}

void IndexBuffer::setCount(unsigned count) {
  this->count = count;
}

const uint16_t *IndexBuffer::getData() const {
  return this->data;
}

void IndexBuffer::setData(uint16_t *data) {
  this->data = data;
}

size_t VertexBuffer::getSize() const {
  return this->size;
}

void VertexBuffer::setSize(size_t size) {
  this->size = size;
}

const uint8_t *VertexBuffer::getData() const {
  return this->data;
}

void VertexBuffer::setData(uint8_t *data) {
  this->data = data;
}
