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

#ifndef MORTAR_RESOURCE_POOL_H
#define MORTAR_RESOURCE_POOL_H

#include <forward_list>
#include <stdexcept>

#include "resource.hpp"

namespace Mortar::Resource {
  template <ResourceType T = Resource>
  class ResourcePool {
    public:
      ~ResourcePool();

      T *getResource();
      void reset();

      friend class ResourceManager;

    protected:
      ResourcePool(std::forward_list<T *> resources)
        : resources { resources },
          iter { this->resources.begin() } {};

      std::forward_list<T *> resources;
      typename std::forward_list<T *>::iterator iter;
  };

  template <ResourceType T>
  ResourcePool<T>::~ResourcePool() {
    for (auto resource : this->resources) {
      delete resource;
    }
  }

  template <ResourceType T>
  T *ResourcePool<T>::getResource() {
    if (this->iter == this->resources.end()) {
      throw std::runtime_error("out of resources in pool");
    }

    T *resource = (*this->iter);
    this->iter++;
    return resource;
  }

  template <ResourceType T>
  void ResourcePool<T>::reset() {
    this->iter = this->resources.begin();
  }
}

#endif
