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

#ifndef MORTAR_RESOURCE_CHARACTER_H
#define MORTAR_RESOURCE_CHARACTER_H

#include <string>
#include <vector>

#include "../layer.hpp"
#include "../joint.hpp"
#include "../model.hpp"
#include "../resource.hpp"

namespace Mortar::Resource::Character {
  class Character : public Resource {
    public:
      Character(ResourceHandle handle)
        : Resource { handle } {};

      const Model *getModel() const;
      void setModel(Mortar::Resource::Model *model);

      void addJoint(Joint *joint);
      const Joint *getJoint(unsigned i) const;
      const std::vector<Joint *>& getJoints() const;

      void addSkinTransform(glm::mat4 skinTransform);
      const glm::mat4& getSkinTransform(unsigned i) const;

      void addLayer(Layer *layer);
      const Layer *getLayer(unsigned i) const;
      const std::vector<Layer *>& getLayers() const;

    private:
      Mortar::Resource::Model *model;
      std::vector<Joint *> joints;
      std::vector<glm::mat4> skinTransforms;
      std::vector<Layer *> layers;
  };
}

#endif
