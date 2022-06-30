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

#ifndef MORTAR_RESOURCE_CHARACTER_DESCRIPTION_H
#define MORTAR_RESOURCE_CHARACTER_DESCRIPTION_H

namespace Mortar::Resource::Character {
  class CharacterDescription {
    public:
      CharacterDescription(const char *name)
        : name {name} {}

      const char *getName();

    private:
      const char *name;
  };
}

#endif
