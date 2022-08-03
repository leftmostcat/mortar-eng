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

#include <SDL2/SDL_rwops.h>
#include <stdint.h>
#include <stdio.h>

class Stream {
  public:
    virtual ~Stream();

    virtual void read(void *ptr, size_t size, size_t count);

    virtual int8_t readInt8();
    virtual int16_t readInt16();
    virtual int32_t readInt32();

    virtual uint8_t readUint8();
    virtual uint16_t readUint16();
    virtual uint32_t readUint32();

    virtual float readFloat();
    virtual char *readString();

    virtual void seek(long offset, int whence);
    virtual long tell();

  protected:
    SDL_RWops *rw;
};

#endif
