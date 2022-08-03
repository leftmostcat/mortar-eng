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

#ifndef MORTAR_RESOURCE_LAYER_H
#define MORTAR_RESOURCE_LAYER_H

#include "mesh.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class Layer : public Resource {
    public:
      Layer(ResourceHandle handle)
        : Resource { handle } {};

      void addDeformableSkinMesh(Mesh *mesh);
      void addKinematicMesh(KinematicMesh *mesh);
      void addSkinMesh(Mesh *mesh);

      const std::vector<Mesh *>& getDeformableSkinMeshes() const;
      const std::vector<KinematicMesh *>& getKinematicMeshes() const;
      const std::vector<Mesh *>& getSkinMeshes() const;

    private:
      std::vector<Mesh *> deformableSkinMeshes;
      std::vector<KinematicMesh *> kinematicMeshes;
      std::vector<Mesh *> skinMeshes;
  };
}

#endif
