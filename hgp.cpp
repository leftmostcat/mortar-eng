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
#include "hgp.hpp"
#include "dds.hpp"
#include "log.hpp"
#include "lsw.hpp"
#include "matrix.hpp"
#include "memorystream.hpp"

using namespace Mortar;

struct HGPHeader {
	uint32_t unk_0000;

	uint32_t strings_offset;
	uint32_t texture_header_offset;
	uint32_t material_header_offset;

	uint32_t unk_0010;

	uint32_t vertex_header_offset;
	uint32_t model_header_offset;

	uint32_t unk_001C[2];

	uint16_t unk_0024[2];

	uint32_t unk_0028;

	uint32_t file_length;
};

struct HGPModelHeader {
	uint32_t unk_0000[5];

	uint32_t mesh_tree_offset;
	uint32_t transformations_offset;
	uint32_t static_transformation_offset;

	uint32_t unk_0020;

	uint32_t layer_header_offset;

	uint32_t unk_0028[2];

	uint32_t strings_offset;

	uint32_t unk_0034;
	float unk_0038;
	uint32_t unk_003C;
	float unk_0040;
	uint32_t unk_0044;
	float unk_0048[9];
	uint32_t unk_006C[4];

	uint8_t num_meshes;
	uint8_t unk_007D;
	uint8_t num_layers;
	uint8_t unk_007F;

	uint32_t unk_0080[13];
};

struct HGPMeshTreeNode {
	glm::mat4 transformation_mtx;

	float unk_0040[4];

	int8_t parent_idx;

	uint8_t unk_0051[3];
	uint32_t unk_0054[3];
};

const uint32_t BODY_OFFSET = 0x30;

HGPModel::HGPModel(Stream &stream) : Model() {
	/* Read in HGP header at the top of the file. */
	stream.seek(0, SEEK_SET);
	struct HGPHeader file_header;

	stream.seek(4, SEEK_CUR);

	file_header.strings_offset = stream.readUint32();
	file_header.texture_header_offset = stream.readUint32();
	file_header.material_header_offset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);

	file_header.vertex_header_offset = stream.readUint32();
	file_header.model_header_offset = stream.readUint32();

	/* Read in additional model information. */
	stream.seek(BODY_OFFSET + file_header.model_header_offset, SEEK_SET);
	struct HGPModelHeader model_header;

	stream.seek(5 * sizeof(uint32_t), SEEK_CUR);

	model_header.mesh_tree_offset = stream.readUint32();
	model_header.transformations_offset = stream.readUint32();
	model_header.static_transformation_offset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);

	model_header.layer_header_offset = stream.readUint32();

	stream.seek(2 * sizeof(uint32_t), SEEK_CUR);

	model_header.strings_offset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);
	stream.seek(sizeof(float), SEEK_CUR);
	stream.seek(sizeof(uint32_t), SEEK_CUR);
	stream.seek(sizeof(float), SEEK_CUR);
	stream.seek(sizeof(uint32_t), SEEK_CUR);
	stream.seek(9 * sizeof(float), SEEK_CUR);
	stream.seek(4 * sizeof(uint32_t), SEEK_CUR);

	model_header.num_meshes = stream.readUint8();

	stream.seek(sizeof(uint8_t), SEEK_CUR);

	model_header.num_layers = stream.readUint8();

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
		int old = stream.tell();

		stream.seek(0x40, SEEK_CUR);

		uint32_t flags = stream.readUint32();

		materials[i].flags = 0;
		if (flags & 0x40000) {
			materials[i].flags |= Model::Material::USE_VERTEX_COLOR;
		}

		stream.seek(0x10, SEEK_CUR);

		materials[i].red = stream.readFloat();
		materials[i].green = stream.readFloat();
		materials[i].blue = stream.readFloat();

		stream.seek(0x14, SEEK_CUR);

		materials[i].alpha = stream.readFloat();

		int16_t texture_idx = stream.readInt16();

		if (texture_idx != -1 && texture_idx & 0x8000)
			materials[i].texture_idx = texture_idx & 0x7FFF;
		else
			materials[i].texture_idx = texture_idx;
	}

	this->setMaterials(materials);

	/* Read mesh tree. */
	stream.seek(BODY_OFFSET + model_header.mesh_tree_offset, SEEK_SET);
	struct HGPMeshTreeNode *tree_nodes = new struct HGPMeshTreeNode[model_header.num_meshes];

	for (int i = 0; i < model_header.num_meshes; i++) {
		tree_nodes[i].transformation_mtx = readMatrix(stream);

		stream.seek(4 * sizeof(float), SEEK_CUR);

		tree_nodes[i].parent_idx = stream.readInt8();

		stream.seek(3 * sizeof(uint8_t), SEEK_CUR);
		stream.seek(3 * sizeof(uint32_t), SEEK_CUR);
	}

	/* Use the mesh tree to apply hierarchical transformations. */
	std::vector<glm::mat4> modelTransforms(model_header.num_meshes);

	stream.seek(BODY_OFFSET + model_header.transformations_offset, SEEK_SET);

	for (int i = 0; i < model_header.num_meshes; i++) {
		glm::mat4 transformMtx = readMatrix(stream);

		modelTransforms[i] = transformMtx;

		if (tree_nodes[i].parent_idx != -1)
			modelTransforms[i] = modelTransforms[i] * modelTransforms[tree_nodes[i].parent_idx];
	}

	delete [] tree_nodes;

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

	/* Read in information necessary for processing layers and meshes. */
	stream.seek(BODY_OFFSET + model_header.static_transformation_offset, SEEK_SET);
	glm::mat4 static_transform_matrix = readMatrix(stream);

	stream.seek(BODY_OFFSET + model_header.layer_header_offset, SEEK_SET);
	struct LSW::LayerHeader *layer_headers = new struct LSW::LayerHeader[model_header.layer_header_offset];

	for (int i = 0; i < model_header.num_layers; i++) {
		layer_headers[i].name_offset = stream.readUint32();

		for (int j = 0; j < 4; j++)
			layer_headers[i].mesh_header_list_offsets[j] = stream.readUint32();
	}

	std::vector<Model::Object> objects;

	/* Break the layers down into meshes and add those to the model's list. */
	for (int i = 0; i < model_header.num_layers; i++) {
		/* XXX: Use model configuration to specify layers by quality. */
		if (i != 0 && i != 1)
			continue;

		for (int j = 0; j < 4; j++) {
			if (!layer_headers[i].mesh_header_list_offsets[j])
				continue;

			if (j % 2 == 0) {
				stream.seek(BODY_OFFSET + layer_headers[i].mesh_header_list_offsets[j], SEEK_SET);
				uint32_t *mesh_header_offsets = new uint32_t[model_header.num_meshes];

				for (int k = 0; k < model_header.num_meshes; k++)
					mesh_header_offsets[k] = stream.readUint32();

				for (int k = 0; k < model_header.num_meshes; k++) {
					Model::Object object;

					object.meshes = LSW::processMeshHeader(stream, BODY_OFFSET, mesh_header_offsets[k], vertexBuffers);
					object.transformation = modelTransforms[k];

					objects.push_back(object);
				}

				delete [] mesh_header_offsets;
			}
			else {
				Model::Object object;

				object.meshes = LSW::processMeshHeader(stream, BODY_OFFSET, layer_headers[i].mesh_header_list_offsets[j], vertexBuffers);
				object.transformation = static_transform_matrix * modelTransforms[0];

				objects.push_back(object);
			}
		}
	}

	delete [] layer_headers;

	this->setObjects(objects);

	/* We have to do this after processing meshes, as stride is stored per-mesh. */
	this->setVertexBuffers(vertexBuffers);
}
