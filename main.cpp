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

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <SDL2/SDL_keycode.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>

#include "game/lsw/game.hpp"
#include "render/gl/renderer.hpp"
#include "log.hpp"
#include "state.hpp"

#define WIDTH 800
#define HEIGHT 600

using namespace Mortar;

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    DEBUG("failed to initialize SDL");
    return -1;
  }

  State::getResourceManager().initialize();

  State::getDisplayManager().initialize(Mortar::DisplayManager::GraphicsAPI::OPENGL, WIDTH, HEIGHT);

  auto renderer = Render::GL::Renderer();
  State::getSceneManager().initialize(&renderer);

  auto game = Game::LSW::Game();
  game.initialize();

  // Start the clock
  State::getClock().initialize();

  // Game loop
  bool shouldClose = false;
  while (!shouldClose) {
    State::getClock().update();
    State::getSceneManager().render();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
          shouldClose = true;
        } else if (event.key.keysym.sym == SDLK_a) {
          State::animEnabled = !State::animEnabled;
        } else if (event.key.keysym.sym == SDLK_r) {
          State::animRate = State::animRate == 30.0f ? 1.0f : 30.0f;
        } else if (event.key.keysym.sym == SDLK_p) {
          State::printNextFrame = true;
        } else if (event.key.keysym.sym == SDLK_i) {
          switch (State::interpolate) {
            case State::InterpolateType::NONE:
              State::interpolate = State::InterpolateType::HERMITE;
              break;
            case State::InterpolateType::HERMITE:
              State::interpolate = State::InterpolateType::NONE;
              break;
          };
        }
      } else if (event.type == SDL_QUIT) {
        shouldClose = true;
      }
    }
  }

  State::getResourceManager().shutDown();
  State::getSceneManager().shutDown();

  SDL_Quit();

  return 0;
}
