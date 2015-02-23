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
#include "texture.h"

Texture::Texture() {
	this->compressed = false;
	this->format = GL_NONE;
	this->internal_format = GL_NONE;
	this->width = 0;
	this->height = 0;

	this->num_levels = 0;
	this->levels = NULL;
}

Texture::~Texture() {
	for (int i = 0; i < this->num_levels; i++) {
		free(this->levels[i].data);
	}

	free(this->levels);
}

void Texture::init(void *texture_data) {}
