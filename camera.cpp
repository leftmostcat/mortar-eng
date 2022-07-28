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

#include "camera.hpp"
#include "math/matrix.hpp"

using namespace Mortar;

void Camera::initialize() {}

void Camera::setPosition(const Math::Vector& position) {
  this->cameraPosition = position;
}

void Camera::setLookAt(const Math::Vector &lookAt) {
  this->cameraLookAt = lookAt;
}

const Math::Matrix Camera::getViewTransform() const {
  return Math::Matrix::lookAt(this->cameraPosition, this->cameraLookAt, Math::Vector::yAxis);
}

void Camera::translate(const Math::Vector &translate) {
  this->cameraPosition = this->cameraPosition + translate;
}
