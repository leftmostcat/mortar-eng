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

#include <string.h>
#include "matrix.h"

Matrix::Matrix() {
	memset(this->array16, 0, 16 * sizeof(float));
}

Matrix Matrix::operator*(const Matrix &right) {
	Matrix result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.array4x4[i][j] = 0;

			for (int k = 0; k < 4; k++) {
				result.array4x4[i][j] += this->array4x4[i][k] * right.array4x4[k][j];
			}
		}
	}

	return result;
}
