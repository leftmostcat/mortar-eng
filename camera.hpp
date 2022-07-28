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

#include "math/matrix.hpp"

namespace Mortar {
  class Camera {
    public:
      void initialize();

      void setPosition(const Math::Vector& position);
      void setLookAt(const Math::Vector& lookAt);
      const Math::Matrix getViewTransform() const;
      void translate(const Math::Vector& translate);

    private:
      Math::Vector cameraLookAt;
      Math::Vector cameraPosition;
  };
}
