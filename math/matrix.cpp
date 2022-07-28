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
  Matrix out;

  out._11 = this->_11 * B._11 + this->_12 * B._21 + this->_13 * B._31 + this->_14 * B._41;
  out._12 = this->_11 * B._12 + this->_12 * B._22 + this->_13 * B._32 + this->_14 * B._42;
  out._13 = this->_11 * B._13 + this->_12 * B._23 + this->_13 * B._33 + this->_14 * B._43;
  out._14 = this->_11 * B._14 + this->_12 * B._24 + this->_13 * B._34 + this->_14 * B._44;

  out._21 = this->_21 * B._11 + this->_22 * B._21 + this->_23 * B._31 + this->_24 * B._41;
  out._22 = this->_21 * B._12 + this->_22 * B._22 + this->_23 * B._32 + this->_24 * B._42;
  out._23 = this->_21 * B._13 + this->_22 * B._23 + this->_23 * B._33 + this->_24 * B._43;
  out._24 = this->_21 * B._14 + this->_22 * B._24 + this->_23 * B._34 + this->_24 * B._44;

  out._31 = this->_31 * B._11 + this->_32 * B._21 + this->_33 * B._31 + this->_34 * B._41;
  out._32 = this->_31 * B._12 + this->_32 * B._22 + this->_33 * B._32 + this->_34 * B._42;
  out._33 = this->_31 * B._13 + this->_32 * B._23 + this->_33 * B._33 + this->_34 * B._43;
  out._34 = this->_31 * B._14 + this->_32 * B._24 + this->_33 * B._34 + this->_34 * B._44;

  out._41 = this->_41 * B._11 + this->_42 * B._21 + this->_43 * B._31 + this->_44 * B._41;
  out._42 = this->_41 * B._12 + this->_42 * B._22 + this->_43 * B._32 + this->_44 * B._42;
  out._43 = this->_41 * B._13 + this->_42 * B._23 + this->_43 * B._33 + this->_44 * B._43;
  out._44 = this->_41 * B._14 + this->_42 * B._24 + this->_43 * B._34 + this->_44 * B._44;

  return out;
}

Matrix Matrix::rotationY(float theta) {
  Matrix out;

  float sinTheta = sin(theta);
  float cosTheta = cos(theta);

  out._11 = cosTheta;
  out._13 = sinTheta;
  out._31 = -sinTheta;
  out._33 = cosTheta;

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

Matrix Matrix::perspectiveRH(float fov, float aspectRatio, float zNear, float zFar) {
  Matrix out;

  float scaleY = 1.0f / tan(fov * 0.5f);
  float zInv = 1.0f / (zFar - zNear);
  float scaleZ = -(zFar + zNear) * zInv;

  out._11 = scaleY * (1.0 / aspectRatio);
  out._22 = scaleY;
  out._33 = scaleZ;
  out._34 = -1.0f;
  out._43 = -2 * zFar * zNear * zInv;
  out._44 = 0.0f;

  return out;
}

Matrix Matrix::lookAt(const Vector& eye, const Vector& at, const Vector& up) {
  Matrix out;

  Vector camDir = at - eye;

  Vector zAxis = Vector::normalize(camDir);
  Vector xAxis = Vector::normalize(Vector::cross(up, zAxis));
  Vector yAxis = Vector::cross(zAxis, xAxis);

  out._11 = xAxis.x;
  out._12 = yAxis.x;
  out._13 = zAxis.x;

  out._21 = xAxis.y;
  out._22 = yAxis.y;
  out._23 = zAxis.y;

  out._31 = xAxis.z;
  out._32 = yAxis.z;
  out._33 = zAxis.z;

  out._41 = -Vector::dot(xAxis, eye);
  out._42 = -Vector::dot(yAxis, eye);
  out._43 = -Vector::dot(zAxis, eye);

  return out;
}

void Matrix::setTranslation(Vector translation) {
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

void Matrix::translate(Vector translation) {
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

Vector Vector::operator-() const {
  Vector out;

  out.x = -this->x;
  out.y = -this->y;
  out.z = -this->z;
  out.w = this->w;

  return out;
}

Vector Vector::operator-(const Vector &b) const {
  Vector out;

  out.x = this->x - b.x;
  out.y = this->y - b.y;
  out.z = this->z - b.z;
  out.w = this->w - b.w;

  return out;
}

Vector Vector::operator+(const Vector &b) const {
  Vector out;

  out.x = this->x + b.x;
  out.y = this->y + b.y;
  out.z = this->z + b.z;
  out.w = this->w + b.w;

  return out;
}

Vector Vector::operator*(const Matrix &M) const {
  Vector out;

  out.x = this->x * M._11 + this->y * M._21 + this->z * M._31 + this->w * M._41;
  out.y = this->x * M._12 + this->y * M._22 + this->z * M._32 + this->w * M._42;
  out.z = this->x * M._13 + this->y * M._23 + this->z * M._33 + this->w * M._43;
  out.w = this->x * M._14 + this->y * M._24 + this->z * M._34 + this->w * M._44;

  return out;
}

Vector Vector::operator*(const float s) const {
  Vector out;

  out.x = this->x * s;
  out.y = this->y * s;
  out.z = this->z * s;
  out.w = this->w;

  return out;
}

float Vector::getAngleFrom(const Vector &v) const {
  float dot = Vector::dot(*this, v);
  return acos(dot / (this->getMagnitude() * v.getMagnitude()));
}

float Vector::getMagnitude() const {
  return sqrt(Vector::dot(*this, *this));
}

std::string Vector::toString() const {
  std::unique_ptr<char []> buf(new char[256]);
  sprintf(buf.get(), "(%.4f, %.4f, %.4f, %.4f)", x, y, z, w);
  size_t length = strlen(buf.get());
  return std::string(buf.get(), buf.get() + length);
}

const Vector Vector::xAxis = Vector { 1.0, 0.0f, 0.0f, 0.0f};
const Vector Vector::yAxis = Vector { 0.0, 1.0f, 0.0f, 0.0f};
const Vector Vector::zAxis = Vector { 0.0, 0.0f, 1.0f, 0.0f};
