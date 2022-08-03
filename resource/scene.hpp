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

#ifndef MORTAR_RESOURCE_SCENE_H
#define MORTAR_RESOURCE_SCENE_H

#include <tsl/sparse_map.h>
#include <vector>

#include "../math/matrix.hpp"
#include "character.hpp"
#include "instance.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "resource.hpp"
#include "spline.hpp"

namespace Mortar::Resource {
  class Scene : public Resource {
    public:
      const Model *getModel() const;
      void setModel(Model *model);

      void addMesh(Mesh *mesh);
      const std::vector<Mesh *>& getMeshes() const;

      void addInstance(Instance *instance);
      const std::vector<Instance *>& getInstances() const;

      void addSpline(const std::string name, const Spline *spline);
      const Spline *getSplineByName(const std::string name) const;

      void addPlayerCharacter(const Character *character);
      const std::vector<const Character *>& getPlayerCharacters() const;

      friend class ResourceManager;

    protected:
      Scene(ResourceHandle handle)
        : Resource { handle } {};

    private:
      Model *model;
      std::vector<Mesh *> meshes;

      std::vector<Instance *> instances;

      std::vector<Math::Matrix> startTransforms;

      tsl::sparse_map<std::string, const Spline *> splines;

      std::vector<const Character *> playerCharacters;
  };
}

#endif
