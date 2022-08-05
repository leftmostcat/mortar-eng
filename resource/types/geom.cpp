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

#include "geom.hpp"

using namespace Mortar::Resource;

void GeomObject::reset() {
  this->mesh = nullptr;
  this->skinTransforms.clear();
  this->worldTransform = Math::Matrix();
}

const Mesh *GeomObject::getMesh() const {
  return this->mesh;
}

void GeomObject::setMesh(const Mortar::Resource::Mesh *mesh) {
  this->mesh = mesh;
}

const Mortar::Math::Matrix& GeomObject::getWorldTransform() const {
  return this->worldTransform;
}

void GeomObject::setWorldTransform(Math::Matrix worldTransform) {
  this->worldTransform = worldTransform;
}

const std::vector<Mortar::Math::Matrix>& GeomObject::getSkinTransforms() const {
  return this->skinTransforms;
}

void GeomObject::setSkinTransforms(std::vector<Math::Matrix> skinTransforms) {
  this->skinTransforms = skinTransforms;
}
