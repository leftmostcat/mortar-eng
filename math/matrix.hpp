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

#ifndef MORTAR_MATH_MATRIX_H
#define MORTAR_MATH_MATRIX_H

#include <array>
#include <math.h>
#include <string>

#include "../log.hpp"
#include "../streams/stream.hpp"

namespace Mortar::Math {
  class Matrix;

  class Vector {
    public:
      Vector(float x, float y, float z, float w)
        : x { x }, y { y }, z { z }, w { w } {};

      Vector()
        : Vector { 0.0f, 0.0f, 0.0f, 0.0f } {};

      Vector operator-() const;
      Vector operator-(const Vector& b) const;
      Vector operator+(const Vector& b) const;
      Vector operator*(const float s) const;

      static inline float dot(const Vector& a, const Vector& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
      }

      static inline Vector cross(const Vector& a, const Vector& b) {
        Vector out;

        out.x = a.y * b.z - a.z * b.y;
        out.y = a.z * b.x - a.x * b.z;
        out.z = a.x * b.y - a.y * b.x;
        out.w = a.w;

        return out;
      }

      static inline Vector normalize(const Vector& v) {
        Vector out;

        float magnitude = v.getMagnitude();
        float invMagnitude = 1.0f / magnitude;

        out.x = v.x * invMagnitude;
        out.y = v.y * invMagnitude;
        out.z = v.z * invMagnitude;
        out.w = v.w;

        return out;
      }

      static inline Vector fromStream(Stream& stream, float w) {
        std::array<float, 4> vec;

        for (int i = 0; i < 3; i++) {
          vec[i] = stream.readFloat();
        }

        return Vector(vec[0], vec[1], vec[2], w);
      }

      const static Vector xAxis;
      const static Vector yAxis;
      const static Vector zAxis;

      float getAngleFrom(const Vector& v) const;
      float getMagnitude() const;

      Vector operator*(const Matrix& M) const;

      std::string toString() const;

      float x;
      float y;
      float z;
      float w;
  };

  class Matrix {
    public:
      Matrix()
        : Matrix {
            { 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f }
          } {};

      Matrix(std::array<float, 16> list);

      static inline Matrix diagonal(float _11, float _22, float _33) {
        std::array<float, 16> mtx {
          _11, 0.0f, 0.0f, 0.0f,
          0.0f, _22, 0.0f, 0.0f,
          0.0f, 0.0f, _33, 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f
        };

        return Matrix(mtx);
      };

      static Matrix rotationY(float theta);
      static Matrix rotationZYX(float alpha, float beta, float gamma);
      static Matrix perspectiveRH(float fov, float aspectRatio, float zNear, float zFar);
      static Matrix lookAt(const Vector& eye, const Vector& at, const Vector& up);

      static inline Matrix fromStream(Stream& stream) {
        std::array<float, 16> mtx;

        for (int i = 0; i < 16; i++) {
          mtx[i] = stream.readFloat();
        }

        return Matrix(mtx);
      }

      void setTranslation(Vector translation);

      void scale(float x, float y, float z);
      void translate (float x, float y, float z);
      void translate(Vector translation);

      void transpose();

      Matrix operator*(const Matrix& B) const;

      const float *operator[](unsigned i) const;

      std::string toString();

      union {
        float m[4][4];
        float f[16];
        struct {
          float _11;
          float _12;
          float _13;
          float _14;
          float _21;
          float _22;
          float _23;
          float _24;
          float _31;
          float _32;
          float _33;
          float _34;
          float _41;
          float _42;
          float _43;
          float _44;
        };
      };
  };
}

#endif
