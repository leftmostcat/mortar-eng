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

#ifndef MORTAR_RESOURCE_INSTANCE_H
#define MORTAR_RESOURCE_INSTANCE_H

#include <forward_list>

#include "../math/matrix.hpp"
#include "mesh.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class Instance : public Resource {
    public:
      Instance(ResourceHandle handle)
        : Resource { handle } {};

      const std::forward_list<Mesh *> getMeshes() const;
      void setMeshes(std::forward_list<Mesh *> mesh);

      const Math::Matrix& getWorldTransform() const;
      void setWorldTransform(Math::Matrix& worldTransform);

    private:
      std::forward_list<Mesh *> meshes;
      Math::Matrix worldTransform;
  };
}

#endif
