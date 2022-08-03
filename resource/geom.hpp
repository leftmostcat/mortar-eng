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

#ifndef MORTAR_RESOURCE_GEOMOBJECT_H
#define MORTAR_RESOURCE_GEOMOBJECT_H

#include <vector>

#include "../math/matrix.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class GeomObject : public Resource {
    public:
      void reset();

      void setMesh(const Mesh *mesh);
      void setWorldTransform(Math::Matrix worldTransform);
      void setSkinTransforms(std::vector<Math::Matrix> skinTransforms);

      const Mesh *mesh;
      Math::Matrix worldTransform;

      std::vector<Math::Matrix> skinTransforms;

      friend class ResourceManager;

    protected:
      GeomObject(ResourceHandle& handle)
        : Resource { handle } {};
  };
}

#endif
