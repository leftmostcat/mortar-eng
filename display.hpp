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

#ifndef MORTAR_DISPLAY_H
#define MORTAR_DISPLAY_H

#include <SDL2/SDL.h>

#include "math/matrix.hpp"

namespace Mortar {
  class DisplayManager {
    public:
      enum class GraphicsAPI {
        OPENGL,
      };

      DisplayManager()
        : aspectRatio { -1.0f },
          fov { (float)std::numbers::pi * 45.0f / 180.0f },
          zNear { 0.15f },
          zFar { 10000.f } {};

      void initialize(GraphicsAPI graphicsApi, unsigned width, unsigned height);

      SDL_Window *getWindow() const;

      void setWindowDimensions(unsigned width, unsigned height);

      const Math::Matrix& getPerspectiveTransform() const;

    private:
      bool isInitialized = false;

      unsigned width;
      unsigned height;
      float aspectRatio;
      float fov;
      float zNear;
      float zFar;

      SDL_Window *window;

      Math::Matrix perspectiveTransform;

      void setManagerDimensions(unsigned width, unsigned height);
  };
}

#endif
