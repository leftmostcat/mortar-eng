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

#include <stdlib.h>
#include <vector>

#include "model.hpp"
#include "texture.hpp"

using namespace Mortar::Resource;

void Model::addTexture(Texture *texture) {
  this->textures.push_back(texture);
}

const std::vector<Texture *>& Model::getTextures() const {
  return this->textures;
}

void Model::addVertexBuffer(VertexBuffer *vertexBuffer) {
  this->vertexBuffers.push_back(vertexBuffer);
}

const std::vector<VertexBuffer *>& Model::getVertexBuffers() const {
  return this->vertexBuffers;
}
