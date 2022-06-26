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

#include "filestream.hpp"

FileStream::FileStream(const char *path, const char *mode) : Stream::Stream() {
  this->rw = SDL_RWFromFile(path, mode);
}

FileStream::~FileStream() {
  SDL_RWclose(this->rw);
}

void FileStream::seek(long offset, int whence) {
  SDL_RWseek(this->rw, offset, whence);
}

long FileStream::tell() {
  return SDL_RWtell(this->rw);
}

void *FileStream::read(size_t size) {
  uint8_t *ptr = new uint8_t[size];

  SDL_RWread(this->rw, ptr, size, 1);

  return (void *)ptr;
}
