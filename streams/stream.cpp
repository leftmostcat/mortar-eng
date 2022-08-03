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

#include "stream.hpp"

Stream::~Stream() {
  if (this->rw != nullptr) {
    SDL_RWclose(this->rw);
  }
}

void Stream::read(void *ptr, size_t size, size_t count) {
  SDL_RWread(this->rw, ptr, size, count);
}

int8_t Stream::readInt8() {
  return SDL_ReadU8(this->rw);
}

int16_t Stream::readInt16() {
  return SDL_ReadLE16(this->rw);
}

int32_t Stream::readInt32() {
  return SDL_ReadLE32(this->rw);
}

uint8_t Stream::readUint8() {
  return SDL_ReadU8(this->rw);
}

uint16_t Stream::readUint16() {
  return SDL_ReadLE16(this->rw);
}

uint32_t Stream::readUint32() {
  return SDL_ReadLE32(this->rw);
}

void Stream::seek(long offset, int whence) {
  SDL_RWseek(this->rw, offset, whence);
}

long Stream::tell() {
  return SDL_RWtell(this->rw);
}

float Stream::readFloat() {
  float val;

  this->read(&val, sizeof(float), 1);

  return val;
}

char *Stream::readString() {
  std::vector<char> vec;

  char val = '\0';
  do {
    this->read(&val, sizeof(char), 1);
    vec.push_back(val);
  } while (val != '\0');

  char *ret = new char[vec.size() + 1];
  memcpy(ret, vec.data(), vec.size());

  return ret;
}
