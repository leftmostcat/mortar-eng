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

#include <stdexcept>

#include "clock.hpp"
#include "display.hpp"
#include "state.hpp"
#include "resource/manager.hpp"
#include "scene/manager.hpp"

using namespace Mortar;

Camera State::camera = Camera();
Clock State::clock = Clock();
Game::Config *State::gameConfig = nullptr;
DisplayManager State::displayManager = DisplayManager();
Resource::ResourceManager State::resourceManager = Resource::ResourceManager();
Scene::SceneManager State::sceneManager = Scene::SceneManager();

float State::animRate = 1.0f;
bool State::animEnabled = true;
bool State::printNextFrame = false;
State::InterpolateType State::interpolate = InterpolateType::HERMITE;

Camera& State::getCamera() {
  return State::camera;
}

Clock& State::getClock() {
  return State::clock;
}

Game::Config *State::getGameConfig() {
  if (!State::gameConfig) {
    throw std::runtime_error("game configuration has not been set");
  }

  return State::gameConfig;
}

DisplayManager& State::getDisplayManager() {
  return State::displayManager;
}

Resource::ResourceManager& State::getResourceManager() {
  return State::resourceManager;
}

Scene::SceneManager& State::getSceneManager() {
  return State::sceneManager;
}

void State::setGameConfig(Game::Config *config) {
  State::gameConfig = config;
}
