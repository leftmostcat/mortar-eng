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

#define GLM_FORCE_RADIANS

#include <array>
#include <math.h>
#include <string>

#include "../streams/stream.hpp"

namespace Mortar::Math {
  class Matrix;

  class Vector {
    public:
      Vector(float x, float y, float z, float w)
        : x { x }, y { y }, z { z }, w { w } {};

      float& operator[](unsigned i) const;

      float x;
      float y;
      float z;
      float w;
  };

  class Point : public Vector {
    public:
      Point(float x, float y, float z)
        : Vector { x, y, z, 1.0f } {};

      Point()
        : Point { 0.0f, 0.0f, 0.0f } {};

      Point operator*(const Matrix& M);
      Point operator-();

      static inline Point fromStream(Stream& stream) {
        float x = stream.readFloat();
        float y = stream.readFloat();
        float z = stream.readFloat();

        return Point(x, y, z);
      }
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

      static Matrix rotationZYX(float alpha, float beta, float gamma);

      static inline Matrix fromStream(Stream& stream) {
        std::array<float, 16> mtx;

        for (int i = 0; i < 16; i++) {
          mtx[i] = stream.readFloat();
        }

        return Matrix(mtx);
      }

      void setTranslation(Point translation);

      void scale(float x, float y, float z);
      void translate (float x, float y, float z);
      void translate(Point translation);

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
