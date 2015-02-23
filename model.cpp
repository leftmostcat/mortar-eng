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

#include <stdlib.h>
#include "model.h"

Model::Model() {
	this->num_vertex_buffers = 0;
	this->num_textures = 0;
	this->num_materials = 0;
	this->num_chunks = 0;

	this->vertex_buffers = NULL;
	this->textures = NULL;
	this->materials = NULL;
	this->chunks = NULL;
}

Model::~Model() {
	for (int i = 0; i < this->num_vertex_buffers; i++) {
		free(this->vertex_buffers[i].ptr);
	}

	for (int i = 0; i < this->num_textures; i++) {
		delete this->textures[i];
	}

	for (int i = 0; i < this->num_chunks; i++) {
		free(this->chunks[i].element_buffer);
	}

	if (this->vertex_buffers)
		free(this->vertex_buffers);

	if (this->textures)
		free(this->textures);

	if (this->chunks)
		free(this->chunks);
}

void Model::load(const char *path) {}
