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

#ifndef MORTAR_RESOURCE_TEXTURE_H
#define MORTAR_RESOURCE_TEXTURE_H

#include <GL/gl.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "resource.hpp"

namespace Mortar::Resource {
  class Texture : public Resource {
    public:
      class Level : public Resource {
        public:
          Level(ResourceHandle handle)
            : Resource { handle } {};

          unsigned getLevel() const;
          void setLevel(unsigned level);

          unsigned getSize() const;
          void setSize(unsigned size);

          uint8_t *getData() const;
          void setData(uint8_t *data);

        private:
          unsigned level;
          unsigned size;
          uint8_t *data;
      };

      Texture(ResourceHandle handle)
        : Resource { handle },
          compressed { false },
          format { GL_NONE },
          internalFormat { GL_NONE },
          width { 0 },
          height { 0 } {};

      bool getIsCompressed() const;
      void setIsCompressed(bool isCompressed);

      GLint getInternalFormat() const;
      void setInternalFormat(GLint internalFormat);

      size_t getWidth() const;
      void setWidth(size_t width);

      size_t getHeight() const;
      void setHeight(size_t height);

      void addLevel(Texture::Level *level);
      const std::vector<Texture::Level *>& getLevels() const;

    private:
      bool compressed;
      GLenum format;
      GLint internalFormat;
      int width;
      int height;

      std::vector<Texture::Level *> levels;
  };
}

#endif
