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

#include <array>
#include <memory>
#include <string.h>

#include "matrix.hpp"

using namespace Mortar::Math;

Matrix::Matrix(std::array<float, 16> list)  {
  std::copy(list.begin(), list.end(), this->f);
};

Matrix Matrix::operator*(const Matrix& B) const {
  // This isn't generalized matrix multiplication, so we make some simplifying
  // assumptions: we ignore column four in rows 1 through 3 because it will
  // consistently be zero and we simply add RHS's row 4 because _44 will
  // consistently be one
  Matrix out;

  out._11 = this->_11 * B._11 + this->_12 * B._21 + this->_13 * B._31;
  out._12 = this->_11 * B._12 + this->_12 * B._22 + this->_13 * B._32;
  out._13 = this->_11 * B._13 + this->_12 * B._23 + this->_13 * B._33;
  out._14 = 0.0f;

  out._21 = this->_21 * B._11 + this->_22 * B._21 + this->_23 * B._31;
  out._22 = this->_21 * B._12 + this->_22 * B._22 + this->_23 * B._32;
  out._23 = this->_21 * B._13 + this->_22 * B._23 + this->_23 * B._33;
  out._24 = 0.0f;

  out._31 = this->_31 * B._11 + this->_32 * B._21 + this->_33 * B._31;
  out._32 = this->_31 * B._12 + this->_32 * B._22 + this->_33 * B._32;
  out._33 = this->_31 * B._13 + this->_32 * B._23 + this->_33 * B._33;
  out._34 = 0.0f;

  out._41 = this->_41 * B._11 + this->_42 * B._21 + this->_43 * B._31 + B._41;
  out._42 = this->_41 * B._12 + this->_42 * B._22 + this->_43 * B._32 + B._42;
  out._43 = this->_41 * B._13 + this->_42 * B._23 + this->_43 * B._33 + B._43;
  out._44 = 1.0f;

  return out;
}

Matrix Matrix::rotationZYX(float alpha, float beta, float gamma) {
  Matrix out;

  float sinAlpha = sin(alpha);
  float cosAlpha = cos(alpha);

  float sinBeta = sin(beta);
  float cosBeta = cos(beta);

  float sinGamma = sin(gamma);
  float cosGamma = cos(gamma);

  out._11 = cosAlpha * cosGamma;
  out._12 = sinAlpha * sinBeta * cosGamma - cosBeta * sinGamma;
  out._13 = sinBeta * sinGamma + sinAlpha * cosBeta * cosGamma;
  out._14 = 0.0f;

  out._21 = cosAlpha * sinGamma;
  out._22 = cosBeta * cosGamma + sinAlpha * sinBeta * sinGamma;
  out._23 = sinAlpha * cosBeta * sinGamma - sinBeta * cosGamma;
  out._24 = 0.0f;

  out._31 = -sinAlpha;
  out._32 = cosAlpha * sinBeta;
  out._33 = cosAlpha * cosBeta;
  out._34 = 0.0f;

  out._41 = 0.0f;
  out._42 = 0.0f;
  out._43 = 0.0f;
  out._44 = 1.0f;

  return out;
}

void Matrix::setTranslation(Point translation) {
  this->_41 = translation.x;
  this->_42 = translation.y;
  this->_43 = translation.z;
}

void Matrix::scale(float x, float y, float z) {
  this->_11 *= x;
  this->_12 *= x;
  this->_13 *= x;

  this->_21 *= y;
  this->_22 *= y;
  this->_23 *= y;

  this->_31 *= z;
  this->_32 *= z;
  this->_33 *= z;
}

void Matrix::translate(float x, float y, float z) {
  this->_41 += x;
  this->_42 += y;
  this->_43 += z;
}

void Matrix::translate(Point translation) {
  this->translate(translation.x, translation.y, translation.z);
}

void Matrix::transpose() {
  float tmp;

  tmp = this->_12;
  this->_12 = this->_21;
  this->_21 = tmp;

  tmp = this->_13;
  this->_13 = this->_31;
  this->_31 = tmp;

  tmp = this->_14;
  this->_14 = this->_41;
  this->_41 = tmp;

  tmp = this->_23;
  this->_23 = this->_32;
  this->_32 = tmp;

  tmp = this->_24;
  this->_24 = this->_42;
  this->_42 = tmp;

  tmp = this->_34;
  this->_34 = this->_43;
  this->_43 = tmp;
}

std::string Matrix::toString() {
  std::unique_ptr<char []> buf(new char[256]);
  sprintf(buf.get(), "[[%.4f, %.4f, %.4f, %.4f];\n[%.4f, %.4f, %.4f, %.4f];\n[%.4f, %.4f, %.4f, %.4f];\n[%.4f, %.4f, %.4f, %.4f]]", _11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44);
  size_t length = strlen(buf.get());
  return std::string(buf.get(), buf.get() + length);
}

Point Point::operator*(const Matrix &M) {
  Point out;

  out.x = this->x * M._11 + this->y * M._21 + this->z * M._31 + M._41;
  out.y = this->x * M._12 + this->y * M._22 + this->z * M._32 + M._42;
  out.z = this->x * M._13 + this->y * M._23 + this->z * M._33 + M._43;
  out.w = 1.0f;

  return out;
}

Point Point::operator-() {
  Point out;

  out.x = -this->x;
  out.y = -this->y;
  out.z = -this->z;
  out.w = 1.0f;

  return out;
}
