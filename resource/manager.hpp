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

#include <forward_list>
#include <functional>
#include <stdexcept>
#include <tsl/sparse_map.h>
#include <vector>

#include "geom.hpp"
#include "pool.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class ResourceManager {
    public:
      void initialize();
      void shutDown();

      template <ResourceType T>
      void registerResourceLoader(ResourceLoader<T> loader);

      template <ResourceType T>
      T *createResource();

      template <ResourceType T>
      ResourcePool<T> *createResourcePool(size_t size);

      template <ResourceType T>
      T *getResource(const std::string& name, bool loadIfAbsent = true);

    private:
      tsl::sparse_map<ResourceHandle, Resource *> resources;
      tsl::sparse_map<std::type_index, ResourceLoader<>> loaders;
      tsl::sparse_map<std::string, Resource *> namedResources;

      std::vector<ResourcePool<> *> resourcePools;
  };

  template <ResourceType T>
  void ResourceManager::registerResourceLoader(ResourceLoader<T> loader) {
    if (loaders.contains(typeid(T))) {
      throw std::runtime_error("resource loader already registered for type");
    }

    loaders[typeid(T)] = loader;
  }

  template <ResourceType T>
  T *ResourceManager::createResource() {
    auto handle = ResourceHandle(typeid(T));

    auto resource = new T(handle);
    this->resources[handle] = resource;

    return resource;
  }

  template <ResourceType T>
  ResourcePool<T> *ResourceManager::createResourcePool(size_t size) {
    std::forward_list<T *> resources (size);

    std::generate(resources.begin(), resources.end(), [] () {
      auto handle = ResourceHandle(typeid(T));

      return new T(handle);
    });

    auto pool = new ResourcePool<T>(resources);

    this->resourcePools.push_back(reinterpret_cast<ResourcePool<> *>(pool));

    return pool;
  }

  template <ResourceType T>
  T *ResourceManager::getResource(const std::string& name, bool loadIfAbsent) {
    if (this->namedResources.contains(name)) {
      return static_cast<T *>(this->namedResources.at(name));
    }

    if (!loadIfAbsent || !this->loaders.contains(typeid(T))) {
      throw std::runtime_error("named resource does not exist and can't be loaded");
    }

    auto loader = this->loaders.at(typeid(T));
    T *resource = static_cast<T *>(loader(name));

    return resource;
  }
}

#endif
