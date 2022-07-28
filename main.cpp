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

#include "game/lsw/config.hpp"
#include "render/gl/renderer.hpp"
#include "math/matrix.hpp"
#include "log.hpp"
#include "state.hpp"
#include "resource/anim.hpp"
#include "resource/character/character.hpp"
#include "resource/character/description.hpp"
#include "resource/providers/lsw/anim.hpp"
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

  State::getResourceManager().initialize();

  State::getDisplayManager().initialize(Mortar::DisplayManager::GraphicsAPI::OPENGL, WIDTH, HEIGHT);

  auto renderer = Render::GL::Renderer();
  State::getSceneManager().initialize(&renderer);

  auto config = Game::LSW::Config();
  State::setGameConfig(&config);

  auto description = Resource::Character::CharacterDescription("obiwankenobi");

  const char *charPath = State::getGameConfig()->getCharacterResourcePath(description.getName());
  auto charStream = FileStream(charPath, "rb");

  Resource::Character::Character *character = Resource::Providers::HGPProvider::read(&description, charPath, charStream);

  const char *animPath = State::getGameConfig()->getAnimationResourcePath("obiwankenobi", "idle");
  auto animStream = FileStream(animPath, "rb");

  Resource::Animation *animation = Resource::Providers::LSW::AnimProvider::read(animPath, animStream);

  character->addSkeletalAnimation(Resource::Character::Character::AnimationType::IDLE, animation);

  State::getSceneManager().addActor(character, Math::Matrix());

  State::getClock().initialize();

  State::getCamera().setPosition(Math::Vector(0.1f, 0.4f, -0.6f, 1.0f));
  State::getCamera().setLookAt(Math::Vector(0.0f, 0.2f, 0.0f, 1.0f));

  /* Main loop. */
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
