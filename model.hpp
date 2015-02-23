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

#ifndef MORTAR_MODEL_H
#define MORTAR_MODEL_H

#include <stdint.h>
#include "matrix.hpp"
#include "texture.hpp"

class Model {
	public:
		explicit Model(void);
		virtual ~Model(void);

		virtual void load(const char *path);

		int num_vertex_buffers;
		int num_chunks;
		int num_textures;
		int num_materials;

		struct VertexBuffer {
			int size;
			int stride;
			float *ptr;
		} *vertex_buffers;

		Texture **textures;

		struct Material {
			float red;
			float green;
			float blue;
			float alpha;
			int texture_idx;
		} *materials;

		struct Chunk {
			int primitive_type;
			int vertex_buffer_idx;
			int material_idx;

			Matrix transformation;

			int num_elements;
			uint16_t *element_buffer;
		} *chunks;
};

#endif
