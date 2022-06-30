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

#include "material.hpp"
#include "texture.hpp"

using namespace Mortar::Resource;

void Material::setFlag(Material::MaterialFlags flag, bool value) {
  if (value) {
    this->flags |= flag;
  } else {
    this->flags &= ~flag;
  }
}

bool Material::isAlphaBlended() const {
  return this->flags & ENABLE_ALPHA_BLEND;
}

void Material::setIsAlphaBlended(bool isAlphaBlended) {
  this->setFlag(Material::MaterialFlags::ENABLE_ALPHA_BLEND, isAlphaBlended);
}

bool Material::isDynamicallyLit() const {
  return this->flags & DYNAMICALLY_LIT;
}

void Material::setIsDynamicallyLit(bool isDynamicallyLit) {
  this->setFlag(Material::MaterialFlags::DYNAMICALLY_LIT, isDynamicallyLit);
}

const float *Material::getColor() const {
  return this->color;
}

void Material::setColor(float red, float green, float blue) {
  this->red = red;
  this->green = green;
  this->blue = blue;
}

const Texture *Material::getTexture() const {
  return this->texture;
}

void Material::setTexture(Texture *texture) {
  this->texture = texture;
}
