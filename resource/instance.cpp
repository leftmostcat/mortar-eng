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

#include "instance.hpp"

using namespace Mortar::Resource;

const std::forward_list<Mesh *> Instance::getMeshes() const {
  return this->meshes;
}

void Instance::setMeshes(std::forward_list<Mesh *> meshes) {
  this->meshes = meshes;
}

const Mortar::Math::Matrix& Instance::getWorldTransform() const {
  return this->worldTransform;
}

void Instance::setWorldTransform(Math::Matrix& worldTransform) {
  this->worldTransform = worldTransform;
}
