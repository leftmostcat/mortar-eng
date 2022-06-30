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
#include <SDL2/SDL.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>

#include "game/lsw/config.hpp"
#include "render/gl/renderer.hpp"
#include "log.hpp"
#include "state.hpp"
#include "resource/character/character.hpp"
#include "resource/character/description.hpp"
#include "resource/providers/hgp.hpp"
#include "streams/filestream.hpp"

#define WIDTH 800
#define HEIGHT 600

using namespace Mortar;

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    DEBUG("failed to initialize SDL");
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow("Mortar Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  if (!window) {
    DEBUG("failed to create window");
    SDL_Quit();
    return -1;
  }

  State::getResourceManager().initialize();

  auto renderer = Render::GL::Renderer(window);
  State::getSceneManager().initialize(&renderer);

  auto config = Game::LSW::Config();
  State::setGameConfig(&config);

  auto description = Resource::Character::CharacterDescription("obiwankenobi");

  const char *path = State::getGameConfig()->getCharacterResourcePath(description.getName());
  auto stream = FileStream(path, "rb");

  Resource::Character::Character *character = Resource::Providers::HGPProvider::read(&description, path, stream);

  State::getSceneManager().addActor(character, glm::identity<glm::mat4>());

  /* Main loop. */
  bool shouldClose = false;
  while (!shouldClose) {
    State::getSceneManager().render();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)) {
        shouldClose = true;
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
