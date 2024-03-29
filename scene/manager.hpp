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

#ifndef MORTAR_SCENE_MANAGER_H
#define MORTAR_SCENE_MANAGER_H

#include <vector>

#include "../resource/pool.hpp"
#include "../resource/types/actor.hpp"
#include "../resource/types/character.hpp"
#include "../resource/types/scene.hpp"
#include "../render/renderer.hpp"

namespace Mortar::Scene {
  class SceneManager {
    public:
      void initialize(Render::Renderer *renderer);
      void shutDown();

      Resource::Actor *addActor(const Resource::Character *character, Math::Matrix worldTransform);
      void setScene(const Resource::Scene *scene);

      void render();

    private:
      Render::Renderer *renderer;
      std::vector<Resource::Actor *> actors;
      const Resource::Scene *scene;
      Resource::ResourcePool<Resource::GeomObject> *geomPool;
  };
}

#endif
