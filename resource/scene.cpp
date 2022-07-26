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

#include "scene.hpp"

using namespace Mortar::Resource;

const Model *Scene::getModel() const {
  return this->model;
}

void Scene::setModel(Model *model) {
  this->model = model;
}

void Scene::addMesh(Mesh *mesh) {
  this->meshes.push_back(mesh);
}

const std::vector<Mesh *>& Scene::getMeshes() const {
  return this->meshes;
}

void Scene::addInstance(Instance *instance) {
  this->instances.push_back(instance);
}

const std::vector<Instance *>& Scene::getInstances() const {
  return this->instances;
}