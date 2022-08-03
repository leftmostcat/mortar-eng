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

#include "texture.hpp"

using namespace Mortar::Resource;

bool Texture::getIsCompressed() const {
  return this->compressed;
}

void Texture::setIsCompressed(bool isCompressed) {
  this->compressed = isCompressed;
}

GLint Texture::getInternalFormat() const {
  return this->internalFormat;
}

void Texture::setInternalFormat(GLint internalFormat) {
  this->internalFormat = internalFormat;
}

size_t Texture::getWidth() const {
  return this->width;
}

void Texture::setWidth(size_t width) {
  this->width = width;
}

size_t Texture::getHeight() const {
  return this->height;
}

void Texture::setHeight(size_t height) {
  this->height = height;
}

void Texture::addLevel(Texture::Level *level) {
  this->levels.push_back(level);
}

const std::vector<Texture::Level *>& Texture::getLevels() const {
  return this->levels;
}

Texture::Level::~Level() {
  delete this->data;
}

unsigned Texture::Level::getLevel() const {
  return this->level;
}

void Texture::Level::setLevel(unsigned level) {
  this->level = level;
}

unsigned Texture::Level::getSize() const {
  return this->size;
}

void Texture::Level::setSize(unsigned size) {
  this->size = size;
}

uint8_t *Texture::Level::getData() const {
  return this->data;
}

void Texture::Level::setData(uint8_t *data) {
  this->data = data;
}
