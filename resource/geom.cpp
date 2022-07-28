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

#include "../math/matrix.hpp"
#include "geom.hpp"

using namespace Mortar::Resource;

void GeomObject::clear() {
  this->next = nullptr;
  this->mesh = nullptr;
  this->worldTransform = Math::Matrix();
}

void GeomObject::setNext(GeomObject *next) {
  this->next = next;
}

void GeomObject::setMesh(const Mortar::Resource::Mesh *mesh) {
  this->mesh = mesh;
}

void GeomObject::setWorldTransform(Math::Matrix worldTransform) {
  this->worldTransform = worldTransform;
}

void GeomObject::setSkinTransforms(std::vector<Math::Matrix> skinTransforms) {
  this->skinTransforms = skinTransforms;
}
