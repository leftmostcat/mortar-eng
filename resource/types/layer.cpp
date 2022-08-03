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

#include "layer.hpp"

using namespace Mortar::Resource;

void Layer::addDeformableSkinMesh(Mesh *mesh) {
  this->deformableSkinMeshes.push_back(mesh);
}

void Layer::addKinematicMesh(KinematicMesh *mesh) {
  this->kinematicMeshes.push_back(mesh);
}

void Layer::addSkinMesh(Mesh *mesh) {
  this->skinMeshes.push_back(mesh);
}

const std::vector<Mortar::Resource::Mesh *>& Layer::getDeformableSkinMeshes() const {
  return this->deformableSkinMeshes;
}

const std::vector<Mortar::Resource::KinematicMesh *>& Layer::getKinematicMeshes() const {
  return this->kinematicMeshes;
}

const std::vector<Mortar::Resource::Mesh *>& Layer::getSkinMeshes() const {
  return this->skinMeshes;
}
