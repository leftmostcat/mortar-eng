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

#include <SDL2/SDL_timer.h>

#include "clock.hpp"

using namespace Mortar;

void Clock::initialize() {
  this->perfFrequency = SDL_GetPerformanceFrequency();
  this->countsLastFrame = SDL_GetPerformanceCounter();

  this->secondsPerCount = 1.0f / this->perfFrequency;
}

void Clock::update() {
  uint64_t counts = SDL_GetPerformanceCounter();

  this->timeDelta = (counts - this->countsLastFrame) * this->secondsPerCount;
  this->countsLastFrame = counts;
}

float Clock::getTimeDelta() {
  return this->timeDelta;
}
