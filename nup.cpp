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
#include "dds.hpp"
#include "lsw.hpp"
#include "memorystream.hpp"
#include "nup.hpp"

using namespace Mortar;

struct NUPHeader {
	uint32_t unk_0000;

	uint32_t strings_offset;
	uint32_t texture_header_offset;
	uint32_t material_header_offset;

	uint32_t unk_0010;

	uint32_t vertex_header_offset;
	uint32_t model_header_offset;
	uint32_t objects_offset;

	uint32_t unk_0020[2];
};

struct NUPModelHeader {
	uint32_t unk_0000[3];

	uint32_t num_materials;
	uint32_t num_meshes;
	uint32_t mesh_header_list_offset;
	uint32_t num_objects;

	uint32_t unk_001C[111];
};

struct NUPObject {
	glm::mat4 transformation;

	uint16_t mesh_idx;
	uint16_t unk_0042;

	uint32_t unk_0044;

	uint32_t matrix_offset;

	uint32_t unk_004C;
};

const int BODY_OFFSET = 0x40;

NUPModel::NUPModel(Stream &stream) : Model() {
	/* Read in NUP header at the top of the file. */
	stream.seek(0, SEEK_SET);
	struct NUPHeader file_header;

	stream.seek(4, SEEK_CUR);

	file_header.strings_offset = stream.readUint32();
	file_header.texture_header_offset = stream.readUint32();
	file_header.material_header_offset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);

	file_header.vertex_header_offset = stream.readUint32();
	file_header.model_header_offset = stream.readUint32();
	file_header.objects_offset = stream.readUint32();

	/* Read in additional model information. */
	stream.seek(BODY_OFFSET + file_header.model_header_offset, SEEK_SET);
	struct NUPModelHeader model_header;

	stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

	model_header.num_materials = stream.readUint32();
	model_header.num_meshes = stream.readUint32();
	model_header.mesh_header_list_offset = stream.readUint32();
	model_header.num_objects = stream.readUint32();

	/* Read texture block information. */
	stream.seek(BODY_OFFSET + file_header.texture_header_offset, SEEK_SET);
	struct LSW::TextureHeader texture_header;

	texture_header.texture_block_offset = stream.readUint32();
	texture_header.texture_block_size = stream.readUint32();
	texture_header.num_textures = stream.readUint32();

	stream.seek(4 * sizeof(uint32_t), SEEK_CUR);
	texture_header.texture_block_headers = new struct LSW::TextureBlockHeader[texture_header.num_textures];

	for (int i = 0; i < texture_header.num_textures; i++) {
		texture_header.texture_block_headers[i].offset = stream.readUint32();

		stream.seek(4 * sizeof(uint32_t), SEEK_CUR);
	}

	/* Read inline DDS textures. */
	std::vector<Texture> textures(texture_header.num_textures);

	for (int i = 0; i < texture_header.num_textures; i++) {
		stream.seek(BODY_OFFSET + file_header.texture_header_offset + 12 + texture_header.texture_block_offset, SEEK_SET);
		stream.seek(texture_header.texture_block_headers[i].offset, SEEK_CUR);

		size_t size;

		/* A rough maximum for file size is calculated from per-texture offsets. */
		if (i < texture_header.num_textures - 1)
			size = texture_header.texture_block_headers[i + 1].offset - texture_header.texture_block_headers[i].offset;
		else
			size = texture_header.texture_block_size - texture_header.texture_block_headers[i].offset;

		uint8_t *texture_data = new uint8_t[size];
		for (int j = 0; j < size; j++)
			texture_data[j] = stream.readUint8();

		MemoryStream ms = MemoryStream(texture_data, size);

		textures[i] = DDSTexture(ms);
	}

	this->setTextures(textures);

	stream.seek(BODY_OFFSET + file_header.material_header_offset, SEEK_SET);
	struct LSW::MaterialHeader material_header;

	material_header.num_materials = stream.readUint32();

	material_header.material_offsets = new uint32_t[material_header.num_materials];

	for (int i = 0; i < material_header.num_materials; i++)
		material_header.material_offsets[i] = stream.readUint32();

	std::vector<Model::Material> materials(material_header.num_materials);

	/* Initialize per-model materials, consisting of a color and index to an in-model texture. */
	for (int i = 0; i < material_header.num_materials; i++) {
		stream.seek(BODY_OFFSET + material_header.material_offsets[i], SEEK_SET);

		stream.seek(21 * sizeof(uint32_t), SEEK_CUR);

		materials[i].red = stream.readFloat();
		materials[i].green = stream.readFloat();
		materials[i].blue = stream.readFloat();

		stream.seek(5 * sizeof(uint32_t), SEEK_CUR);

		materials[i].alpha = stream.readFloat();

		int16_t texture_idx = stream.readInt16();

		if (texture_idx != -1 && texture_idx & 0x8000)
			materials[i].texture_idx = texture_idx & 0x7FFF;
		else
			materials[i].texture_idx = texture_idx;
	}

	this->setMaterials(materials);

	/* Read the vertex information header. */
	stream.seek(BODY_OFFSET + file_header.vertex_header_offset, SEEK_SET);
	struct LSW::VertexHeader vertex_header;

	vertex_header.num_vertex_blocks = stream.readUint32();

	stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

	vertex_header.blocks = new struct LSW::VertexBlock[vertex_header.num_vertex_blocks];

	for (int i = 0; i < vertex_header.num_vertex_blocks; i++) {
		vertex_header.blocks[i].size = stream.readUint32();
		vertex_header.blocks[i].id = stream.readUint32();
		vertex_header.blocks[i].offset= stream.readUint32();
	}

	/* Read vertex blocks into individual, indexed buffers. */
	std::vector<Model::VertexBuffer> vertexBuffers(vertex_header.num_vertex_blocks);

	for (int i = 0; i < vertex_header.num_vertex_blocks; i++) {
		vertexBuffers[i].size = vertex_header.blocks[i].size;

		stream.seek(BODY_OFFSET + file_header.vertex_header_offset + vertex_header.blocks[i].offset, SEEK_SET);
		vertexBuffers[i].data = new uint8_t[vertexBuffers[i].size];

		for (int j = 0; j < vertexBuffers[i].size; j++)
			vertexBuffers[i].data[j] = stream.readUint8();
	}

	std::vector<std::vector<Model::Mesh> > meshes(model_header.num_meshes);

	/* Break the layers down into meshes and add those to the model's list. */
	stream.seek(BODY_OFFSET + model_header.mesh_header_list_offset, SEEK_SET);
	uint32_t *mesh_header_offsets = new uint32_t[model_header.num_meshes];

	for (int i = 0; i < model_header.num_meshes; i++)
		mesh_header_offsets[i] = stream.readUint32();

	for (int i = 0; i < model_header.num_meshes; i++) {
		meshes[i] = LSW::processMesh(stream, BODY_OFFSET, mesh_header_offsets[i], vertexBuffers);
	}

	delete mesh_header_offsets;

	/* We have to do this after processing meshes, as stride is stored per-mesh. */
	this->setVertexBuffers(vertexBuffers);

	stream.seek(BODY_OFFSET + file_header.objects_offset, SEEK_SET);
	NUPObject *objects_data = new NUPObject[model_header.num_objects];

	for (int i = 0; i < model_header.num_objects; i++) {
		objects_data[i].transformation = readMatrix(stream);
		objects_data[i].mesh_idx = stream.readUint16();

		stream.seek(sizeof(uint16_t), SEEK_CUR);
		stream.seek(sizeof(uint32_t), SEEK_CUR);

		objects_data[i].matrix_offset = stream.readUint32();

		stream.seek(sizeof(uint32_t), SEEK_CUR);
	}

	std::vector<Model::Object> objects(model_header.num_objects);

	for (int i = 0; i < model_header.num_objects; i++) {
		if (objects_data[i].matrix_offset) {
			stream.seek(BODY_OFFSET + objects_data[i].matrix_offset, SEEK_SET);
			objects[i].transformation = readMatrix(stream);
		}
		else {
			objects[i].transformation = objects_data[i].transformation;
		}

		objects[i].meshes = meshes[objects_data[i].mesh_idx];
	}

	delete objects_data;

	this->setObjects(objects);
}
