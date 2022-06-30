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

#include <stdexcept>

#include "manager.hpp"

using namespace Mortar::Resource;

void ResourceManager::initialize() {
  this->geomObjectPool.resize(ResourceManager::MAX_GEOMS);
  for (size_t i = 0; i < ResourceManager::MAX_GEOMS; i++) {
    this->geomObjectPool[i] = new GeomObject();
  }

  this->geomObjectPoolIter = this->geomObjectPool.begin();
}

void ResourceManager::shutDown() {
  for (auto geomObject = this->geomObjectPool.begin(); geomObject != this->geomObjectPool.end(); geomObject++) {
    delete *geomObject;
  }
}

GeomObject *ResourceManager::getResource() {
  if (this->geomObjectPoolIter == this->geomObjectPool.end()) {
    throw std::runtime_error("out of free geom objects");
  }

  GeomObject *geom = (*this->geomObjectPoolIter);
  this->geomObjectPoolIter++;

  return geom;
}

void ResourceManager::clearGeomObjectPool() {
  this->geomObjectPoolIter = this->geomObjectPool.begin();
}
