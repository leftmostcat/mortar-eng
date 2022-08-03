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

#include "../../state.hpp"
#include "game.hpp"
#include "loaders/loaders.hpp"
#include "../../resource/manager.hpp"

using namespace Mortar::Game::LSW;

void Game::initialize() {
  Resource::ResourceManager& resourceManager = State::getResourceManager();

  resourceManager.registerResourceLoader<Resource::Character>(CharacterLoader());
  resourceManager.registerResourceLoader<Resource::Scene>(SceneLoader());

  Scene::SceneManager& sceneManager = State::getSceneManager();

  // Load a scene we know we have
  Resource::Scene *scene = resourceManager.getResource<Resource::Scene>("negotiations_a");
  sceneManager.setScene(scene);
}
