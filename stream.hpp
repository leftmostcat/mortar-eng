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

#ifndef MORTAR_STREAM_H
#define MORTAR_STREAM_H

#include <stdint.h>
#include <stdio.h>

class Stream {
	public:
		int8_t readInt8();
		int16_t readInt16();
		int32_t readInt32();

		uint8_t readUint8();
		uint16_t readUint16();
		uint32_t readUint32();

		float readFloat();

		char *readString();

		virtual void seek(long offset, int whence) = 0;
		virtual long tell() = 0;

	private:
		virtual void *read(size_t size) = 0;
};

#endif
