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
#include "dds.hpp"
#include "log.hpp"

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

DDSTexture::DDSTexture(Stream &stream) : Texture() {
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

	std::vector<Texture::Level> levels(file_header.num_levels);

	stream.seek(128, SEEK_SET);

	if (file_header.format.flags & DDS_HAS_FOURCC) {
		switch (file_header.format.fourCC) {
			case DDS_FORMAT_DXT3:
				this->compressed = true;
				this->format = GL_BGRA;
				this->internal_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

				/* Read in mipmap levels one by one. */
				for (int i = 0; i < file_header.num_levels; i++) {
					levels[i].size = (((file_header.width >> i) + 3) / 4) * (((file_header.height >> i) + 3) / 4) * 16;

					levels[i].data = new uint8_t[levels[i].size];

					for (int j = 0; j < levels[i].size; j++)
						levels[i].data[j] = stream.readUint8();
				}

				DEBUG("texture has %d levels", file_header.num_levels);
				break;
			default:
				fprintf(stderr, "Unrecognized fourCC: %d\n", file_header.format.fourCC);
				return;
		}
	}

	this->width = file_header.width;
	this->height = file_header.height;
	this->levels = levels;
}
