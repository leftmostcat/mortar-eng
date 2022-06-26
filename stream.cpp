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

int8_t Stream::readInt8() {
  int8_t *val = (int8_t *)this->read(1);

  return *val;
}

int16_t Stream::readInt16() {
  int16_t *val = (int16_t *)this->read(2);

  return *val;
}

int32_t Stream::readInt32() {
  int32_t *val = (int32_t *)this->read(4);

  return *val;
}

uint8_t Stream::readUint8() {
  uint8_t *val = (uint8_t *)this->read(1);

  return *val;
}

uint16_t Stream::readUint16() {
  uint16_t *val = (uint16_t *)this->read(2);

  return *val;
}

uint32_t Stream::readUint32() {
  uint32_t *val = (uint32_t *)this->read(4);

  return *val;
}

float Stream::readFloat() {
  float *val = (float *)this->read(4);

  return *val;
}

char *Stream::readString() {
  std::vector<char> vec;
  char *val = (char *)this->read(1);

  while (*val != '\0') {
    vec.push_back(*val);

    val = (char *)this->read(1);
  }

  char *ret = new char[vec.size() + 1];

  for (int i = 0; i < vec.size(); i++) {
    ret[i] = vec[i];
  }

  ret[vec.size()] = '\0';

  return ret;
}
