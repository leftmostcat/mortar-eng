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

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "geom.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class ResourceManager {
    public:
      void initialize();
      void shutDown();

      template <ResourceType T>
      T *createResource();

      GeomObject *getResource();
      void clearGeomObjectPool();

    private:
      static constexpr size_t MAX_GEOMS = 4096;

      std::vector<Resource *> resources;
      std::vector<GeomObject *> geomObjectPool;
      std::vector<GeomObject *>::iterator geomObjectPoolIter;
  };

  template <ResourceType T>
  T *ResourceManager::createResource() {
    auto handle = ResourceHandle(typeid(T));

    auto resource = new T(handle);

    // unordered_map feels like the obvious choice to use here, but insertion
    // massively tanks performance and we don't need to retrieve by handle
    // often, so a sorted vector works out better
    auto upper = std::upper_bound(
      this->resources.begin(),
      this->resources.end(),
      resource,
      [] (const Resource *a, const Resource *b) {
        return a->handle.id < b->handle.id;
      }
    );

    this->resources.insert(upper, resource);

    return resource;
  }
}

#endif
