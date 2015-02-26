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

#include <vector>
#include <stdint.h>
#include "matrix.hpp"
#include "texture.hpp"

class Model {
	public:
		class VertexBuffer {
			public:
				int size;
				int stride;
				uint8_t *data;
		};

		class Material {
			public:
				union {
					struct {
						float red;
						float green;
						float blue;
						float alpha;
					};
					float color[4];
				};

				int texture_idx;
		};

		class Chunk {
			public:
				int primitive_type;
				int vertex_buffer_idx;
				int material_idx;

				glm::mat4 transformation;

				int num_elements;
				uint16_t *element_buffer;
		};

		void setVertexBuffers(std::vector<Model::VertexBuffer> vertexBuffers);
		Model::VertexBuffer getVertexBuffer(int i);
		int getVertexBufferCount();

		void setTextures(std::vector<Texture> textures);
		Texture getTexture(int i);
		int getTextureCount();

		void setMaterials(std::vector<Model::Material> materials);
		Model::Material getMaterial(int i);

		void addChunk(Model::Chunk chunk);
		Model::Chunk getChunk(int i);
		int getChunkCount();

	private:
		std::vector<Model::VertexBuffer> vertexBuffers;
		std::vector<Texture> textures;
		std::vector<Model::Material> materials;
		std::vector<Model::Chunk> chunks;
};

#endif
