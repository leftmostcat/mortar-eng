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

#include <string.h>
#include "memorystream.hpp"

MemoryStream::MemoryStream(void *data, size_t size) : Stream() {
  this->data = (uint8_t *)data;
  this->size = size;
  this->pos = 0;
}

void MemoryStream::seek(long offset, int whence) {
  switch (whence) {
    case SEEK_SET:
      this->pos = offset;
      break;
    case SEEK_CUR:
      this->pos += offset;
      break;
    case SEEK_END:
      this->pos = this->size - offset;
      break;
  }
}

long MemoryStream::tell() {
  return this->pos;
}

void *MemoryStream::read(size_t size) {
  uint8_t *ptr = new uint8_t[size];
  int remaining = this->size - this->pos;
  int to_copy = 0;

  if (remaining >= size) {
    to_copy = size;
  }
  else if (remaining > 0) {
    to_copy = remaining;
  }

  int to_fill = size - to_copy;

  memcpy(ptr, this->data + this->pos, to_copy);
  memset(ptr + to_copy, 0, to_fill);

  this->pos += to_copy;

  return (void *)ptr;
}
