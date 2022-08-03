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
#include <unordered_map>
#include <vector>

#include "../../math/matrix.hpp"
#include "../anim.hpp"
#include "../layer.hpp"
#include "../joint.hpp"
#include "../model.hpp"
#include "../resource.hpp"

namespace Mortar::Resource::Character {
  class Character : public Resource {
    public:
      enum class AnimationType {
        NONE,
        IDLE,
      };

      class Locator : public Resource {
        public:
          Locator(ResourceHandle handle)
            : Resource { handle } {};

          const Math::Matrix& getTransform() const;
          void setTransform(const Math::Matrix& transform);

          unsigned char getJointIdx() const;
          void setJointIdx(unsigned char jointIdx);

        private:
          Math::Matrix transform;
          unsigned char jointIdx;
      };

      Character(ResourceHandle handle)
        : Resource { handle } {};

      const Model *getModel() const;
      void setModel(Mortar::Resource::Model *model);

      void addJoint(Joint *joint);
      const Joint *getJoint(unsigned i) const;
      const std::vector<Joint *>& getJoints() const;

      void addSkinTransform(Math::Matrix& skinTransform);
      const Math::Matrix& getSkinTransform(unsigned i) const;

      const std::vector<Math::Matrix>& getRestPose() const;
      void setRestPose(std::vector<Math::Matrix>& restPose);

      void addLayer(Layer *layer);
      const Layer *getLayer(unsigned i) const;
      const std::vector<Layer *>& getLayers() const;

      void addLocator(Locator *locator);
      const Locator *getLocatorFromExternalIdx(unsigned char idx) const;

      void addExternalLocatorMapping(unsigned char external, unsigned char internal);

      void addSkeletalAnimation(AnimationType type, Animation *animation);
      bool hasSkeletalAnimation(AnimationType type) const;
      const Animation *getSkeletalAnimation(AnimationType type) const;

    private:
      Mortar::Resource::Model *model;
      std::vector<Joint *> joints;
      std::vector<Math::Matrix> restPose;
      std::vector<Math::Matrix> skinTransforms;
      std::vector<Layer *> layers;
      std::vector<Locator *> locators;
      std::unordered_map<unsigned char, unsigned char> externalLocatorMap;
      std::unordered_map<AnimationType, Animation *> skeletalAnimations;
  };
}

#endif
