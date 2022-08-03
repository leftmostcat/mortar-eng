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

#include <filesystem>
#include <stdexcept>
#include <string>
#include <tsl/sparse_map.h>

#include "../../../state.hpp"
#include "../../../streams/filestream.hpp"
#include "loaders.hpp"
#include "../readers/nup.hpp"

using namespace Mortar::Game::LSW;

const std::filesystem::path levelsDir { std::filesystem::path(dataPath).append("levels") };
const std::filesystem::path ep1Dir { std::filesystem::path(levelsDir).append("episode_i") };

struct SceneDescription {
  std::filesystem::path path;
  std::string filePrefix;

  std::vector<std::string> playerCharacters;
};

tsl::sparse_map<std::string, struct SceneDescription> sceneDescriptions = {
  {
    "negotiations_a",
    {
      std::filesystem::path(ep1Dir).append("chapter_01/negotiations_a"),
      "negotiations_a",
      { "quigonjinn", "obiwankenobi" }
    }
  }
};

Mortar::Resource::Scene *SceneLoader::operator()(const std::string &name) {
  if (!sceneDescriptions.contains(name)) {
    throw std::runtime_error("unknown scene name");
  }

  Mortar::Resource::Scene *resource = State::getResourceManager().createResource<Mortar::Resource::Scene>();

  struct SceneDescription& desc = sceneDescriptions.at(name);

  auto nupPath = std::filesystem::path(desc.path).append(desc.filePrefix).concat(".nup");
  FileStream stream = FileStream(nupPath.c_str(), "rb");

  Readers::NUPReader::read(resource, stream);

  for (auto& charName : desc.playerCharacters) {
    Resource::Character *pc = State::getResourceManager().getResource<Resource::Character>(charName);
    resource->addPlayerCharacter(pc);
  }

  return resource;
}
