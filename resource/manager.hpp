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

#ifndef MORTAR_RESOURCE_MANAGER_H
#define MORTAR_RESOURCE_MANAGER_H

#include <map>
#include <stdexcept>
#include <vector>

#include "geom.hpp"

namespace Mortar::Resource {
  class ResourceManager {
    public:
      void initialize();
      void shutDown();

      template <class T> T *getResource(const char *name);
      GeomObject *getResource();
      void clearGeomObjectPool();

    private:
      static constexpr size_t MAX_GEOMS = 4096;

      std::map<ResourceHandle, Resource *> resources;
      std::vector<GeomObject *> geomObjectPool;
      std::vector<GeomObject *>::iterator geomObjectPoolIter;
  };

  template <class T> T *ResourceManager::getResource(const char *name) {
    static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");

    if (this->resources.contains(name)) {
      T *resource = dynamic_cast<T *>(this->resources.at(name));
      if (resource == nullptr) {
        throw std::runtime_error("resource of different type already exists");
      }

      return resource;
    }

    T *resource = new T(name);
    this->resources[name] = resource;

    return resource;
  }
}

#endif
