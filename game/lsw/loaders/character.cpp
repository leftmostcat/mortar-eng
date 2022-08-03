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
#include <tsl/sparse_map.h>

#include "../../../state.hpp"
#include "../../../streams/filestream.hpp"
#include "loaders.hpp"
#include "../readers/anim.hpp"
#include "../readers/hgp.hpp"

using namespace Mortar::Game::LSW;

const std::filesystem::path charsDir { std::filesystem::path(dataPath).append("chars") };

struct CharacterDescription {
  std::filesystem::path path;
  std::string filePrefix;

  tsl::sparse_map<Mortar::Resource::Character::AnimationType, std::string> animations;
};

tsl::sparse_map<std::string, struct CharacterDescription> charDescriptions = {
  {
    "quigonjinn",
    {
      std::filesystem::path(charsDir).append("quigonjinn"),
      "quigonjinn",
      {
        { Mortar::Resource::Character::AnimationType::IDLE, "idle" }
      }
    }
  },
  {
    "obiwankenobi",
    {
      std::filesystem::path(charsDir).append("obiwankenobi"),
      "obiwankenobi",
      {
        { Mortar::Resource::Character::AnimationType::IDLE, "idle" }
      }
    }
  }
};

Mortar::Resource::Character *CharacterLoader::operator()(const std::string &name) {
  if (!charDescriptions.contains(name)) {
    throw std::runtime_error("unknown scene name");
  }

  Mortar::Resource::Character *resource = State::getResourceManager().createResource<Mortar::Resource::Character>();

  struct CharacterDescription& desc = charDescriptions.at(name);

  auto hgpPath = std::filesystem::path(desc.path).append(desc.filePrefix).concat(".hgp");
  FileStream stream = FileStream(hgpPath.c_str(), "rb");

  Readers::HGPReader::read(resource, stream);

  for (auto& animation : desc.animations) {
    auto animPrefix = std::filesystem::path(desc.path).append(animation.second);

    auto aniPath = std::filesystem::path(animPrefix).concat(".ani");
    FileStream stream = FileStream(aniPath.c_str(), "rb");

    Mortar::Resource::Animation *ani = Readers::AnimReader::read(stream);
    resource->addSkeletalAnimation(animation.first, ani);
  }

  return resource;
}
