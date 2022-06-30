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

#ifndef MORTAR_RESOURCE_JOINT_H
#define MORTAR_RESOURCE_JOINT_H

#include "matrix.hpp"
#include "resource.hpp"

namespace Mortar::Resource {
  class Joint : public Resource {
    public:
      Joint(ResourceHandle handle)
        : Resource { handle } {};

      const char *getName() const;
      void setName(const char *name);

      size_t getParentIdx() const;
      void setParentIdx(size_t parentIdx);

      const glm::mat4& getTransform() const;
      void setTransform(glm::mat4& transform);

      const glm::mat4& getRestPoseTransform() const;
      void setRestPoseTransform(glm::mat4& transform);

    private:
      const char *name;
      size_t parentIdx;
      glm::mat4 transform;
      glm::mat4 restPoseTransform;
  };
}

#endif
