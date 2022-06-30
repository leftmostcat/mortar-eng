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

using namespace Mortar::Resource::Character;

const Mortar::Resource::Model *Character::getModel() const {
  return this->model;
}

void Character::setModel(Mortar::Resource::Model *model) {
  this->model = model;
}

void Character::addJoint(Joint *joint) {
  this->joints.push_back(joint);
}

const Mortar::Resource::Joint *Character::getJoint(unsigned i) const {
  return this->joints.at(i);
}

const std::vector<Mortar::Resource::Joint *>& Character::getJoints() const {
  return this->joints;
}

void Character::addSkinTransform(glm::mat4 skinTransform) {
  this->skinTransforms.push_back(skinTransform);
}

const glm::mat4& Character::getSkinTransform(unsigned i) const {
  return this->skinTransforms.at(i);
}

void Character::addLayer(Layer *layer) {
  this->layers.push_back(layer);
}

const Mortar::Resource::Layer *Character::getLayer(unsigned i) const {
  return this->layers.at(i);
}

const std::vector<Mortar::Resource::Layer *>& Character::getLayers() const {
  return this->layers;
}
