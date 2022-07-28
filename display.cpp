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

#include <SDL2/SDL_video.h>
#include <stdexcept>

#include "display.hpp"
#include "log.hpp"
#include "math/matrix.hpp"

using namespace Mortar;

void DisplayManager::initialize(GraphicsAPI graphicsApi, unsigned int width, unsigned int height) {
  if (width == 0 || height == 0) {
    throw std::runtime_error("height and width must be positive");
  }

  this->setManagerDimensions(width, height);

  unsigned windowFlags = 0;
  switch (graphicsApi) {
    case GraphicsAPI::OPENGL:
      windowFlags |= SDL_WINDOW_OPENGL;
  };

  this->window = SDL_CreateWindow("Mortar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);

  this->isInitialized = true;
}

SDL_Window *DisplayManager::getWindow() const {
  if (!this->isInitialized) {
    throw std::runtime_error("not initialized");
  }

  return this->window;
}

const Math::Matrix& DisplayManager::getPerspectiveTransform() const {
  if (!this->isInitialized) {
    throw std::runtime_error("not initialized");
  }

  return this->perspectiveTransform;
}

void DisplayManager::setWindowDimensions(unsigned width, unsigned height) {
  if (!this->isInitialized) {
    throw std::runtime_error("not initialized");
  }

  this->setManagerDimensions(width, height);

  SDL_SetWindowSize(this->window, width, height);
}

void DisplayManager::setManagerDimensions(unsigned width, unsigned height) {
  this->width = width;
  this->height = height;

  float newAspectRatio = (float)width / height;
  if (this->aspectRatio != newAspectRatio) {
    this->aspectRatio = newAspectRatio;

    this->perspectiveTransform = Math::Matrix::perspectiveRH(this->fov, this->aspectRatio, this->zNear, this->zFar);
  }
}
