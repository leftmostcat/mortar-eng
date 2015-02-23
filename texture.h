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

#ifndef MORTAR_TEXTURE_H
#define MORTAR_TEXTURE_H

#include <GL/gl.h>

class Texture {
	public:
		explicit Texture(void);
		virtual ~Texture(void);

		virtual void init(void *texture_data);

		bool compressed;
		GLenum format;
		GLint internal_format;
		int width;
		int height;

		int num_levels;

		struct Level {
			int size;
			void *data;
		} *levels;
};

#endif
