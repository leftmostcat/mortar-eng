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

#include "character.hpp"
#include "scene.hpp"

using namespace Mortar::Resource;

const Model *Scene::getModel() const {
  return this->model;
}

void Scene::setModel(Model *model) {
  this->model = model;
}

void Scene::addInstance(Instance *instance) {
  this->instances.push_back(instance);
}

const std::vector<Instance *>& Scene::getInstances() const {
  return this->instances;
}

void Scene::addSpline(const std::string name, const Spline *spline) {
  this->splines[name] = spline;
}

const Spline *Scene::getSplineByName(const std::string name) const {
  if (!this->splines.contains(name)) {
    return nullptr;
  }

  return this->splines.at(name);
}

void Scene::addPlayerCharacter(const Character *character) {
  this->playerCharacters.push_back(character);
}

const std::vector<const Character *>& Scene::getPlayerCharacters() const {
  return this->playerCharacters;
}
