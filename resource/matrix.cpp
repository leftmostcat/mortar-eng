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

#include "matrix.hpp"

glm::mat4 readMatrix(Stream &stream) {
  float mtx_array[16];

  /* D3DMATRIX is row-major and we need column-major for OpenGL, so read funny. */
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mtx_array[i + j * 4] = stream.readFloat();
    }
  }

  return glm::make_mat4(mtx_array);
}
