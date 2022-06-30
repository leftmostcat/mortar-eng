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

#include <stdio.h>
#include <stdint.h>

#include "../../log.hpp"
#include "../../state.hpp"
#include "../texture.hpp"
#include "dds.hpp"

using namespace Mortar::Resource::Providers;

#define MKTAG(a, b, c, d) ((uint32_t)((a) | ((b) << 8) | ((c) << 16) | ((d) << 24)))

#define DDS_FORMAT_DXT1 MKTAG('D', 'X', 'T', '1')
#define DDS_FORMAT_DXT3 MKTAG('D', 'X', 'T', '3')
#define DDS_FORMAT_DXT5 MKTAG('D', 'X', 'T', '5')

enum DDSFormatFlags {
  DDS_HAS_ALPHA = 0x01,
  DDS_HAS_FOURCC = 0x04,
  DDS_IS_INDEXED = 0x20,
  DDS_IS_RGB = 0x40
};

struct DDSPixelFormat {
  uint32_t size;
  uint32_t flags;
  uint32_t fourCC;
  uint32_t num_bits;
  uint32_t red_bitmask;
  uint32_t green_bitmask;
  uint32_t blue_bitmask;
  uint32_t alpha_bitmask;
};

struct DDSHeader {
  uint32_t tag;
  uint32_t header_size;
  uint32_t flags;
  uint32_t height;
  uint32_t width;
  uint32_t pitch;
  uint32_t depth;
  uint32_t num_levels;

  uint32_t reserved[11];

  struct DDSPixelFormat format;

  uint32_t caps;
  uint32_t caps2;
  uint32_t caps3;
  uint32_t caps4;
  uint32_t reserved2;
};

Mortar::Resource::Texture *DDSProvider::read(const char *name, Stream &stream) {
  ResourceManager resourceManager = State::getResourceManager();
  Texture *texture = resourceManager.getResource<Texture>(name);

  struct DDSHeader file_header;

  file_header.tag = stream.readUint32();
  file_header.header_size = stream.readUint32();
  file_header.flags = stream.readUint32();
  file_header.height = stream.readUint32();
  file_header.width = stream.readUint32();

  /* Pitch and depth are currently unused. */
  stream.seek(2 * sizeof(uint32_t), SEEK_CUR);

  file_header.num_levels = stream.readUint32();

  stream.seek(11 * sizeof(uint32_t), SEEK_CUR);

  file_header.format.size = stream.readUint32();
  file_header.format.flags = stream.readUint32();
  file_header.format.fourCC = stream.readUint32();

  stream.seek(128, SEEK_SET);

  if (file_header.format.flags & DDS_HAS_FOURCC) {
    switch (file_header.format.fourCC) {
      case DDS_FORMAT_DXT3:
        texture->setIsCompressed(true);
        // texture->format = GL_BGRA;
        texture->setInternalFormat(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);

        /* Read in mipmap levels one by one. */
        for (int i = 0; i < file_header.num_levels; i++) {
          // XXX: Breaks if we have more than 99 levels
          char *levelName = (char *)calloc(strlen(name) + 9, sizeof(char));
          sprintf(levelName, "%s.level%.2d", name, i);
          Texture::Level *level = resourceManager.getResource<Texture::Level>(levelName);

          level->setLevel(i);
          level->setSize((((file_header.width >> i) + 3) >> 2) * (((file_header.height >> i) + 3) >> 2) * 16);

          uint8_t *data = new uint8_t[level->getSize()];

          for (int j = 0; j < level->getSize(); j++) {
            data[j] = stream.readUint8();
          }

          level->setData(data);

          texture->addLevel(level);
        }

        DEBUG("texture has %d levels", file_header.num_levels);
        break;
      default:
        fprintf(stderr, "Unrecognized fourCC: %d\n", file_header.format.fourCC);
        return nullptr;
    }
  }

  texture->setWidth(file_header.width);
  texture->setHeight(file_header.height);

  return texture;
}
