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

#ifndef MORTAR_GAME_LSW_LOADERS_H
#define MORTAR_GAME_LSW_LOADERS_H

#include <filesystem>

#include "../../../resource/types/character.hpp"
#include "../../../resource/types/scene.hpp"

namespace Mortar::Game::LSW {
  const std::filesystem::path dataPath { "lego_data" };

  class CharacterLoader {
    public:
      Resource::Character *operator()(const std::string& name);
  };

  class SceneLoader {
    public:
      Resource::Scene *operator()(const std::string& name);
  };
}

#endif
