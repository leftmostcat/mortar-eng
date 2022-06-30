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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.hpp"

using namespace Mortar::Game::LSW;

const char *Config::getCharacterResourcePath(const char *name) {
  char pathBuf[1024];
  sprintf(pathBuf, "%s/Chars/%s/%s.hgp", this->dataPath, name, name);

  char *path = (char *)calloc(strlen(pathBuf) + 1, sizeof(char));
  strcpy(path, pathBuf);

  return path;
}

const char *Config::getSceneResourcePath(const char *name) {
  char pathBuf[1024];
  sprintf(pathBuf, "%s/Levels/%s/%s.nup", this->dataPath, name, name);

  char *path = (char *)calloc(strlen(pathBuf) + 1, sizeof(char));
  strcpy(path, pathBuf);

  return path;
}
