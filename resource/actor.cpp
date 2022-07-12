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

#include "character/character.hpp"
#include "actor.hpp"

using namespace Mortar::Resource;

const Character::Character *Actor::getCharacter() const {
  return this->character;
}

void Actor::setCharacter(Character::Character *character) {
  this->character = character;
}

const Mortar::Math::Matrix& Actor::getWorldTransform() const {
  return this->worldTransform;
}

void Actor::setWorldTransform(Math::Matrix &worldTransform) {
  this->worldTransform = worldTransform;
}

Character::Character::AnimationType Actor::getAnimation() const {
  return this->animState.animType;
}

void Actor::setAnimation(Character::Character::AnimationType animType) {
  this->animState.animType = animType;
  this->animState.position = 0.0f;
}

void Actor::setAnimation(Character::Character::AnimationType animType, float position) {
  this->animState.animType = animType;
  this->animState.position = position;
}

void Actor::advanceAnimation(float timeDelta) {
  this->animState.position += timeDelta;

  const Animation *anim = character->getSkeletalAnimation(this->animState.animType);
  if (!anim) {
    this->animState.position = 0.0f;
    return;
  }

  // XXX: Ignores animations that can't loop, animations that can play
  // backwards, etc.
  float animLength = anim->getLength();
  if (this->animState.position > animLength) {
    float difference = this->animState.position - animLength;
    this->animState.position = difference;
  }
}

float Actor::getAnimationPosition() const {
  return this->animState.position;
}
