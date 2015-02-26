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
#include "matrix.hpp"
#include "memorystream.hpp"

#define BODY_OFFSET 0x30
#define OFFSET(off) (body + off)

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

struct HGPVertexBlock {
	uint32_t size;
	uint32_t id;
	uint32_t offset;
};

struct HGPVertexHeader {
	uint32_t num_vertex_blocks;

	uint32_t unk_0004[3];

	struct HGPVertexBlock *blocks;
};

struct HGPTextureBlockHeader {
	uint32_t offset;

	uint32_t unk_0004[4];
};

struct HGPTextureHeader {
	uint32_t texture_block_offset;
	uint32_t texture_block_size;
	uint32_t num_textures;

	uint32_t unk_000C[4];

	struct HGPTextureBlockHeader *texture_block_headers;
};

struct HGPLayerHeader {
	uint32_t name_offset;
	uint32_t mesh_header_list_offsets[4];
};

struct HGPMeshHeader {
	uint32_t unk_0000[3];

	uint32_t mesh_offset;

	uint32_t unk_0010;
};

struct HGPMesh {
	uint32_t next_offset;

	uint32_t unk_0004;

	uint32_t material_idx;
	uint32_t vertex_type;

	uint32_t unk_0010[3];

	uint32_t vertex_buffer_idx;

	uint32_t unk_0020[4];

	uint32_t chunk_offset;

	uint32_t unk_0034[4];
};

struct HGPChunk {
	uint32_t next_offset;
	uint32_t primitive_type;

	uint16_t num_elements;
	uint16_t unk_000A;

	uint32_t elements_offset;

	uint32_t unk_0010[16];
};

struct HGPMaterialHeader {
	uint32_t num_materials;
	uint32_t *material_offsets;
};

struct HGPMaterial {
	uint32_t unk_0000[21];

	float red;
	float green;
	float blue;

	uint32_t unk_0060[5];

	uint32_t alpha;

	int16_t texture_idx;
	uint16_t unk_007A;

	uint32_t unk_007C[14];
};

struct HGPMeshTreeNode {
	glm::mat4 transformation_mtx;

	float unk_0040[4];

	int8_t parent_idx;

	uint8_t unk_0051[3];
	uint32_t unk_0054[3];
};

static glm::mat4 readMatrix(Stream &stream) {
	float mtx_array[16];

	/* D3DMATRIX is row-major and we need column-major for OpenGL, so read funny. */
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mtx_array[i + j * 4] = stream.readFloat();

	return glm::make_mat4(mtx_array);
}

static struct HGPMesh readMeshInfo(Stream &stream, uint32_t mesh_offset) {
	stream.seek(BODY_OFFSET + mesh_offset, SEEK_SET);
	struct HGPMesh mesh;

	mesh.next_offset = stream.readUint32();

	stream.seek(sizeof(uint32_t), SEEK_CUR);

	mesh.material_idx = stream.readUint32();
	mesh.vertex_type = stream.readUint32();

	stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

	mesh.vertex_buffer_idx = stream.readUint32();

	stream.seek(4 * sizeof(uint32_t), SEEK_CUR);

	mesh.chunk_offset = stream.readUint32();

	return mesh;
}

static struct HGPChunk readChunkInfo(Stream &stream, uint32_t chunk_offset) {
	stream.seek(BODY_OFFSET + chunk_offset, SEEK_SET);
	struct HGPChunk chunk;

	chunk.next_offset = stream.readUint32();
	chunk.primitive_type = stream.readUint32();

	chunk.num_elements = stream.readUint16();

	stream.seek(sizeof(uint16_t), SEEK_CUR);

	chunk.elements_offset = stream.readUint32();

	return chunk;
}

void HGPModel::processMesh(Stream &stream, uint32_t mesh_header_offset, glm::mat4 transform, std::vector<Model::VertexBuffer> &vertexBuffers) {
	if (!mesh_header_offset)
		return;

	stream.seek(BODY_OFFSET + mesh_header_offset, SEEK_SET);
	struct HGPMeshHeader mesh_header;

	stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

	mesh_header.mesh_offset = stream.readUint32();

	if (!mesh_header.mesh_offset)
		return;

	struct HGPMesh mesh = readMeshInfo(stream, mesh_header.mesh_offset);
	bool processNextMesh;

	do {
		int stride = 0;

		/* Vertex stride is specified per-mesh. */
		switch (mesh.vertex_type) {
			case 89:
				stride = 36;
				break;
			case 93:
				stride = 56;
				break;
			default:
				fprintf(stderr, "Unknown vertex type %d\n", mesh.vertex_type);
		}

		int vertex_buffer_idx = mesh.vertex_buffer_idx - 1;

		vertexBuffers[vertex_buffer_idx].stride = stride;

		struct HGPChunk chunk_data = readChunkInfo(stream, mesh.chunk_offset);
		bool processNextChunk;

		do {
			Model::Chunk chunk = Model::Chunk();

			chunk.vertex_buffer_idx = vertex_buffer_idx;
			chunk.material_idx = mesh.material_idx;
			chunk.primitive_type = chunk_data.primitive_type;
			chunk.num_elements = chunk_data.num_elements;

			chunk.transformation = transform;

			chunk.element_buffer = new uint16_t[chunk_data.num_elements];

			stream.seek(BODY_OFFSET + chunk_data.elements_offset, SEEK_SET);

			for (int i = 0; i < chunk_data.num_elements; i++)
				chunk.element_buffer[i] = stream.readUint16();

			this->addChunk(chunk);

			processNextChunk = false;
			if (chunk_data.next_offset) {
				chunk_data = readChunkInfo(stream, chunk_data.next_offset);
				processNextChunk = true;
			}
		} while (processNextChunk);

		processNextMesh = false;
		if (mesh.next_offset) {
			mesh = readMeshInfo(stream, mesh.next_offset);
			processNextMesh = true;
		}
	} while (processNextMesh);
}

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
	struct HGPTextureHeader texture_header;

	texture_header.texture_block_offset = stream.readUint32();
	texture_header.texture_block_size = stream.readUint32();
	texture_header.num_textures = stream.readUint32();

	stream.seek(4 * sizeof(uint32_t), SEEK_CUR);
	texture_header.texture_block_headers = new struct HGPTextureBlockHeader[texture_header.num_textures];

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

		textures[i] = DDSTexture::DDSTexture(ms);
	}

	this->setTextures(textures);

	stream.seek(BODY_OFFSET + file_header.material_header_offset, SEEK_SET);
	struct HGPMaterialHeader material_header;

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
		int offset = i * 16 * sizeof(float);
		glm::mat4 transformMtx = readMatrix(stream);

		modelTransforms[i] = transformMtx;

		if (tree_nodes[i].parent_idx != -1)
			modelTransforms[i] = modelTransforms[i] * modelTransforms[tree_nodes[i].parent_idx];
	}

	/* Read the vertex information header. */
	stream.seek(BODY_OFFSET + file_header.vertex_header_offset, SEEK_SET);
	struct HGPVertexHeader vertex_header;

	vertex_header.num_vertex_blocks = stream.readUint32();

	stream.seek(3 * sizeof(uint32_t), SEEK_CUR);

	vertex_header.blocks = new struct HGPVertexBlock[vertex_header.num_vertex_blocks];

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
	struct HGPLayerHeader *layer_headers = new struct HGPLayerHeader[model_header.layer_header_offset];

	for (int i = 0; i < model_header.num_layers; i++) {
		layer_headers[i].name_offset = stream.readUint32();

		for (int j = 0; j < 4; j++)
			layer_headers[i].mesh_header_list_offsets[j] = stream.readUint32();
	}

	/* Break the layers down into meshes and add those to the model's list. */
	for (int i = 0; i < model_header.num_layers; i++) {
		/* XXX: Use model configuration to specify layers by quality. */
		if (i != 0 && i != 2)
			continue;

		for (int j = 0; j < 4; j++) {
			if (!layer_headers[i].mesh_header_list_offsets[j])
				continue;

			if (j % 2 == 0) {
				stream.seek(BODY_OFFSET + layer_headers[i].mesh_header_list_offsets[j], SEEK_SET);
				uint32_t *mesh_header_offsets = new uint32_t[model_header.num_meshes];

				for (int k = 0; k < model_header.num_meshes; k++)
					mesh_header_offsets[k] = stream.readUint32();

				for (int k = 0; k < model_header.num_meshes; k++)
					this->processMesh(stream, mesh_header_offsets[k], modelTransforms[k], vertexBuffers);
			}
			else
				this->processMesh(stream, layer_headers[i].mesh_header_list_offsets[j], static_transform_matrix * modelTransforms[0], vertexBuffers);
		}
	}

	/* We have to do this after processing meshes, as stride is stored per-mesh. */
	this->setVertexBuffers(vertexBuffers);
}
