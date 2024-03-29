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

#ifndef MORTAR_RESOURCE_ACTOR_H
#define MORTAR_RESOURCE_ACTOR_H

#include "../../math/matrix.hpp"
#include "../resource.hpp"
#include "character.hpp"

namespace Mortar::Resource {
  class Actor : public Resource {
    public:
      const Character *getCharacter() const;
      void setCharacter(const Character *character);

      const Math::Matrix& getWorldTransform() const;
      void setWorldTransform(Math::Matrix& worldTransform);

      Character::AnimationType getAnimation() const;
      void setAnimation(Character::AnimationType animType);
      void setAnimation(Character::AnimationType animType, float position);

      void advanceAnimation(float timeDelta);
      float getAnimationPosition() const;

      friend class ResourceManager;

    protected:
      Actor(ResourceHandle& handle)
        : Resource { handle } {};

    private:
      class AnimState {
        public:
          float position;
          Character::Character::AnimationType animType;
      };

      AnimState animState;
      const Character *character;
      Math::Matrix worldTransform;
  };
}

#endif
