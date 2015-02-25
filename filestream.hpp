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

#ifndef MORTAR_FILESTREAM_H
#define MORTAR_FILESTREAM_H

#include "stream.hpp"

class FileStream : public Stream {
	public:
		explicit FileStream(const char *path, const char *mode);
		~FileStream();

		virtual void seek(long offset, int whence);
		virtual long tell();

	private:
		virtual void *read(size_t size);

		FILE *fp;
};

#endif
