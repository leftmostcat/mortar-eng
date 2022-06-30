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

#include <SDL2/SDL_rwops.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filestream.hpp"
#include "../log.hpp"

FileStream::FileStream(const char *path, const char *mode) : Stream::Stream() {
  if (path == NULL) {
    throw std::ifstream::failure("path must not be null");
  }

  size_t pathlen = strlen(path);

  char *pathcopy = (char *)calloc(pathlen + 1, sizeof(char));
  if (!pathcopy) {
    throw std::ifstream::failure("unable to allocate memory");
  }

  strncpy(pathcopy, path, pathlen);

  char *newpath = (char *)calloc(pathlen + 1, sizeof(char));
  if (!newpath) {
    free(pathcopy);
    throw std::ifstream::failure("unable to allocate memory");
  }

  char *nextpart = newpath;

  char *strtok_saveptr = NULL;
  char *token = strtok_r(pathcopy, "/", &strtok_saveptr);

  if (token == NULL) {
    free(newpath);
    free(pathcopy);
    throw std::ifstream::failure("path must not be empty");
  }

  DIR *dir;
  if (pathcopy[0] == '/') {
    dir = opendir("/");

    strcpy(nextpart, "/");
    nextpart++;
  } else {
    dir = opendir("./");
  }

  try {
    do {
      struct dirent *content = readdir(dir);

      bool found = false;
      while (content != NULL) {
        if (strcasecmp(token, content->d_name) == 0) {
            found = true;
            break;
        }

        content = readdir(dir);
      }

      if (!found) {
        throw std::ifstream::failure("file or directory not found");
      }

      strcpy(nextpart, content->d_name);
      nextpart += strlen(content->d_name);

      struct stat results;

      if (stat(newpath, &results) != 0) {
        throw std::ifstream::failure("cannot access file or directory");
      }

      token = strtok_r(NULL, "/", &strtok_saveptr);

      if (S_ISREG(results.st_mode)) {
        if (token != NULL) {
          throw std::ifstream::failure("file or directory not found");
        }

        free(pathcopy);
        closedir(dir);

        this->rw = SDL_RWFromFile(newpath, mode);

        free(newpath);

        break;
      } else if (S_ISDIR(results.st_mode)) {
        if (token == NULL) {
          throw std::ifstream::failure("path is not a file");
        }

        closedir(dir);
        dir = opendir(newpath);

        nextpart[0] = '/';
        nextpart++;
      } else {
        throw std::ifstream::failure("file or directory not found");
      }
    } while (true);
  } catch (std::ifstream::failure& exception) {
    free(pathcopy);
    free(newpath);
    closedir(dir);

    throw;
  }
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
