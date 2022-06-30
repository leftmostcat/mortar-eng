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

#include "game/config.hpp"
#include "resource/manager.hpp"
#include "scene/manager.hpp"

namespace Mortar {
  class State {
    public:
      static Game::Config *getGameConfig();
      static Resource::ResourceManager& getResourceManager();
      static Scene::SceneManager& getSceneManager();
      static void setGameConfig(Game::Config *config);

    private:
      static Game::Config *gameConfig;
      static Resource::ResourceManager resourceManager;
      static Scene::SceneManager sceneManager;
  };
}

#endif
