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

#include "../../math/matrix.hpp"
#include "../resource.hpp"

namespace Mortar::Resource {
  class Joint : public Resource {
    public:
      Joint(ResourceHandle handle)
        : Resource { handle } {};

      const char *getName() const;
      void setName(const char *name);

      int getParentIdx() const;
      void setParentIdx(int parentIdx);

      const Math::Matrix& getTransform() const;
      void setTransform(Mortar::Math::Matrix& transform);

      const Math::Vector& getAttachmentPoint() const;
      void setAttachmentPoint(Math::Vector& attachmentPoint);

      bool getIsRelativeToAttachment() const;
      void setIsRelativeToAttachment(bool isRelativeToAttachment);

    private:
      enum Flags {
        IS_RELATIVE_TO_ATTACHMENT = 1 << 0,
      };

      void setFlag(Flags flag, bool value);

      const char *name;
      int parentIdx;
      Math::Matrix transform;
      Math::Vector attachmentPoint;

      unsigned char flags;
  };
}

#endif
