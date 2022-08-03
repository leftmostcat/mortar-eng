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

#ifndef MORTAR_STATE_H
#define MORTAR_STATE_H

#include "camera.hpp"
#include "clock.hpp"
#include "display.hpp"
#include "resource/manager.hpp"
#include "scene/manager.hpp"

namespace Mortar {
  class State {
    public:
      static Clock& getClock();
      static Camera& getCamera();
      static DisplayManager& getDisplayManager();
      static Resource::ResourceManager& getResourceManager();
      static Scene::SceneManager& getSceneManager();

      enum class InterpolateType {
        NONE,
        HERMITE,
      };

      static float animRate;
      static bool animEnabled;
      static bool printNextFrame;
      static InterpolateType interpolate;

    private:
      static Camera camera;
      static Clock clock;
      static DisplayManager displayManager;
      static Resource::ResourceManager resourceManager;
      static Scene::SceneManager sceneManager;
  };
}

#endif
