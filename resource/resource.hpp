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

#ifndef MORTAR_RESOURCE_H
#define MORTAR_RESOURCE_H

#include <functional>
#include <typeindex>

#include "../log.hpp"

namespace Mortar::Resource {
  // ResourceHandle is an opaque unique identifier for resources
  class ResourceHandle {
    public:
      bool operator==(const ResourceHandle& other) const;

      friend std::hash<ResourceHandle>;
      friend class ResourceManager;

    private:
      // Restrict construction of ResourceHandles; only the ResourceManager
      // should be creating new ones
      ResourceHandle(std::type_index type)
        : id { ResourceHandle::nextId++ }, type { type } {};

      static std::size_t nextId;

      std::size_t id;
      std::type_index type;
  };

  class Resource {
    public:
      Resource(ResourceHandle& handle)
        : handle { handle } {};

      virtual ~Resource() {}

      const ResourceHandle& getHandle() const;

      friend class ResourceManager;

    private:
      ResourceHandle handle;
  };

  template <typename T>
  concept ResourceType = std::derived_from<T, Resource>;
}

// Specialize std::hash for ResourceHandle to allow use as map key
template <>
struct std::hash<Mortar::Resource::ResourceHandle> {
  std::size_t operator()(const Mortar::Resource::ResourceHandle& handle) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(handle.id);
    std::size_t h2 = std::hash<std::type_index>{}(handle.type);

    return h1 ^ (h2 << 1);
  }
};

#endif
