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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hgp.hpp"
#include "dds.hpp"
#include "matrix.hpp"

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

	uint32_t unk_001C;
	uint32_t unk_0020;

	uint16_t unk_0024;
	uint16_t unk_0026;

	uint32_t unk_0028;

	uint32_t file_length;
};

struct HGPModelHeader {
	uint32_t unk_0000;
	uint32_t unk_0004;
	uint32_t unk_0008;
	uint32_t unk_000C;
	uint32_t unk_0010;

	uint32_t mesh_tree_offset;
	uint32_t transformations_offset;
	uint32_t static_transformations_offset;

	uint32_t unk_0020;

	uint32_t layer_header_offset;

	uint32_t unk_0028;
	uint32_t unk_002C;

	uint32_t strings_offset;

	uint32_t unk_0034;

	float unk_0038;

	uint32_t unk_003C;

	float unk_0040;

	uint32_t unk_0044;

	float unk_0048;
	float unk_004C;
	float unk_0050;
	float unk_0054;
	float unk_0058;
	float unk_005C;
	float unk_0060;
	float unk_0064;
	float unk_0068;

	uint32_t unk_006C;
	uint32_t unk_0070;
	uint32_t unk_0074;
	uint32_t unk_0078;

	uint8_t num_meshes;
	uint8_t unk_007D;
	uint8_t num_layers;
	uint8_t unk_007F;

	uint32_t unk_0080;
	uint32_t unk_0084;
	uint32_t unk_0088;
	uint32_t unk_008C;
	uint32_t unk_0090;
	uint32_t unk_0094;
	uint32_t unk_0098;
	uint32_t unk_009C;
	uint32_t unk_00A0;
	uint32_t unk_00A4;
	uint32_t unk_00A8;
	uint32_t unk_00AC;
	uint32_t unk_00B0;
};

struct HGPVertexHeader {
	uint32_t num_vertex_blocks;

	uint32_t unk_0004;
	uint32_t unk_0008;
	uint32_t unk_000C;

	struct {
		uint32_t size;
		uint32_t id;
		uint32_t offset;
	} blocks[];
};

struct HGPTextureBlockHeader {
	uint32_t offset;

	uint32_t unk_0004;
	uint32_t unk_0008;
	uint32_t unk_000C;
	uint32_t unk_0010;
};

struct HGPTextureHeader {
	uint32_t texture_block_offset;
	uint32_t texture_block_size;
	uint32_t num_textures;

	uint32_t unk_000C;
	uint32_t unk_0010;
	uint32_t unk_0014;
	uint32_t unk_0018;
	struct HGPTextureBlockHeader texture_block_headers[];
};

struct HGPLayerHeader {
	uint32_t name_offset;
	uint32_t mesh_header_list_offsets[4];
};

struct HGPMeshHeader {
	uint32_t unk_0000;
	uint32_t unk_0004;
	uint32_t unk_0008;

	uint32_t mesh_offset;

	uint32_t unk_0010;
};

struct HGPMesh {
	uint32_t next_offset;

	uint32_t unk_0004;

	uint32_t material_idx;
	uint32_t vertex_type;

	uint32_t unk_0010;
	uint32_t unk_0014;
	uint32_t unk_0018;

	uint32_t vertex_buffer_idx;

	uint32_t unk_0020;
	uint32_t unk_0024;
	uint32_t unk_0028;
	uint32_t unk_002C;

	uint32_t chunk_offset;

	uint32_t unk_0034;
	uint32_t unk_0038;
	uint32_t unk_003C;
	uint32_t unk_0040;
};

struct HGPChunk {
	uint32_t next_offset;
	uint32_t primitive_type;

	uint16_t num_elements;
	uint16_t unk_000A;

	uint32_t elements_offset;

	uint32_t unk_0010;
	uint32_t unk_0014;
	uint32_t unk_0018;
	uint32_t unk_001C;
	uint32_t unk_0020;
	uint32_t unk_0024;
	uint32_t unk_0028;
	uint32_t unk_002C;
	uint32_t unk_0030;
	uint32_t unk_0034;
	uint32_t unk_0038;
	uint32_t unk_003C;
	uint32_t unk_0040;
	uint32_t unk_0044;
	uint32_t unk_0048;
	uint32_t unk_004C;
};

struct HGPMaterialHeader {
	uint32_t num_materials;
	uint32_t material_offsets[];
};

struct HGPMaterial {
	uint32_t unk_0000;
	uint32_t unk_0004;
	uint32_t unk_0008;
	uint32_t unk_000C;
	uint32_t unk_0010;
	uint32_t unk_0014;
	uint32_t unk_0018;
	uint32_t unk_001C;
	uint32_t unk_0020;
	uint32_t unk_0024;
	uint32_t unk_0028;
	uint32_t unk_002C;
	uint32_t unk_0030;
	uint32_t unk_0034;
	uint32_t unk_0038;
	uint32_t unk_003C;
	uint32_t unk_0040;
	uint32_t unk_0044;
	uint32_t unk_0048;
	uint32_t unk_004C;
	uint32_t unk_0050;

	float red;
	float green;
	float blue;

	uint32_t unk_0060;
	uint32_t unk_0064;
	uint32_t unk_0068;
	uint32_t unk_006C;
	uint32_t unk_0070;

	uint32_t alpha;

	int16_t texture_idx;
	uint16_t unk_007A;

	uint32_t unk_007C;
	uint32_t unk_0080;
	uint32_t unk_0084;
	uint32_t unk_0088;
	uint32_t unk_008C;
	uint32_t unk_0090;
	uint32_t unk_0094;
	uint32_t unk_0098;
	uint32_t unk_009C;
	uint32_t unk_00A0;
	uint32_t unk_00A4;
	uint32_t unk_00A8;
	uint32_t unk_00AC;
	uint32_t unk_00B0;
};

struct HGPMeshTreeNode {
	Matrix transformation_mtx;

	float unk_0040[3];
	float unk_004C;

	int8_t parent_idx;
	uint8_t unk_0051;
	uint8_t unk_0052;
	uint8_t unk_0053;

	uint32_t unk_0054;
	uint32_t unk_0058;
	uint32_t unk_005C;
};

void HGPModel::processMesh(char *body, uint32_t mesh_header_offset, Matrix transform) {
	if (!mesh_header_offset)
		return;

	struct HGPMeshHeader *mesh_header = (struct HGPMeshHeader *)OFFSET(mesh_header_offset);

	if (!mesh_header->mesh_offset)
		return;

	struct HGPMesh *mesh = (struct HGPMesh *)OFFSET(mesh_header->mesh_offset);

	do {
		struct HGPChunk *chunk = (struct HGPChunk *)OFFSET(mesh->chunk_offset);

		do {
			int idx = this->num_chunks++;
			/* XXX: Don't reallocate on each chunk. */
			this->chunks = (struct Chunk *)realloc(this->chunks, sizeof(struct Chunk) * this->num_chunks);

			this->chunks[idx].vertex_buffer_idx = mesh->vertex_buffer_idx - 1;
			this->chunks[idx].material_idx = mesh->material_idx;
			this->chunks[idx].primitive_type = chunk->primitive_type;
			this->chunks[idx].num_elements = chunk->num_elements;

			/* Vertex stride is specified per-mesh. */
			/* XXX: We shouldn't set stride every chunk. */
			switch (mesh->vertex_type) {
				case 89:
					this->vertex_buffers[this->chunks[idx].vertex_buffer_idx].stride = 36;
					break;
				case 93:
					this->vertex_buffers[this->chunks[idx].vertex_buffer_idx].stride = 56;
					break;
				default:
					this->vertex_buffers[this->chunks[idx].vertex_buffer_idx].stride = 0;
					fprintf(stderr, "Unknown vertex type %d\n", mesh->vertex_type);
			}

			this->chunks[idx].transformation = transform;

			int element_buffer_size = chunk->num_elements * sizeof(uint16_t);
			this->chunks[idx].element_buffer = (uint16_t *)malloc(element_buffer_size);
			memcpy(this->chunks[idx].element_buffer, OFFSET(chunk->elements_offset), element_buffer_size);
		} while (chunk->next_offset && (chunk = (struct HGPChunk *)OFFSET(chunk->next_offset)));
	} while (mesh->next_offset && (mesh = (struct HGPMesh *)OFFSET(mesh->next_offset)));
}

void HGPModel::load(const char *path) {
	/* Initial file read into memory. */
	FILE *hgp = fopen(path, "rb");

	if (fseek(hgp, 0, SEEK_END) == -1) {
		int err = errno;
		fprintf(stderr, "%s\n", strerror(err));
		return;
	}
	long length = ftell(hgp);
	rewind(hgp);

	char *buf = (char *)malloc(length);
	fread(buf, 1, length, hgp);
	fclose(hgp);

	/* Locate file body and basic headers. */
	char *body = buf + BODY_OFFSET;
	struct HGPHeader *file_header = (struct HGPHeader *)buf;
	struct HGPModelHeader *model_header = (struct HGPModelHeader *)OFFSET(file_header->model_header_offset);

	/* Read vertex blocks into individual, indexed buffers. */
	struct HGPVertexHeader *vertex_header = (struct HGPVertexHeader *)OFFSET(file_header->vertex_header_offset);
	this->num_vertex_buffers = vertex_header->num_vertex_blocks;
	this->vertex_buffers = (struct VertexBuffer *)calloc(this->num_vertex_buffers, sizeof(struct VertexBuffer));

	for (int i = 0; i < vertex_header->num_vertex_blocks; i++) {
		this->vertex_buffers[i].size = vertex_header->blocks[i].size;

		this->vertex_buffers[i].ptr = (float *)malloc(this->vertex_buffers[i].size);
		memcpy(this->vertex_buffers[i].ptr, OFFSET(file_header->vertex_header_offset + vertex_header->blocks[i].offset), this->vertex_buffers[i].size);
	}

	/* Read inline DDS textures. */
	struct HGPTextureHeader *texture_header = (struct HGPTextureHeader *)OFFSET(file_header->texture_header_offset);
	this->num_textures = texture_header->num_textures;
	this->textures = (Texture **)calloc(this->num_textures, sizeof(Texture *));

	for (int i = 0; i < texture_header->num_textures; i++) {
		this->textures[i] = new DDSTexture();
		void *texture_data = (void *)OFFSET(file_header->texture_header_offset + texture_header->texture_block_offset + 12 + texture_header->texture_block_headers[i].offset);
		this->textures[i]->init(texture_data);
	}

	/* Initialize per-model materials, consisting of a color and index to an in-model texture. */
	struct HGPMaterialHeader *material_header = (struct HGPMaterialHeader *)OFFSET(file_header->material_header_offset);

	std::vector<Model::Material> materials(material_header->num_materials);

	for (int i = 0; i < material_header->num_materials; i++) {
		struct HGPMaterial *material = (struct HGPMaterial *)OFFSET(material_header->material_offsets[i]);

		materials[i].red = material->red;
		materials[i].green = material->green;
		materials[i].blue = material->blue;
		materials[i].alpha = material->alpha;

		if (material->texture_idx != -1 && material->texture_idx & 0x8000)
			materials[i].texture_idx = material->texture_idx & 0x7FFF;
		else
			materials[i].texture_idx = material->texture_idx;
	}

	this->setMaterials(materials);

	/* Use the mesh tree to apply hierarchical transformations. */
	struct HGPMeshTreeNode *tree_nodes = (struct HGPMeshTreeNode *)OFFSET(model_header->mesh_tree_offset);
	Matrix *transform_matrices = (Matrix *)OFFSET(model_header->transformations_offset);
	Matrix *static_transform_matrices = (Matrix *)OFFSET(model_header->static_transformations_offset);
	Matrix *model_transforms = (Matrix *)calloc(model_header->num_meshes, sizeof(Matrix));

	for (int i = 0; i < model_header->num_meshes; i++) {
		model_transforms[i] = transform_matrices[i];

		if (tree_nodes[i].parent_idx != -1)
			model_transforms[i] = model_transforms[i] * model_transforms[tree_nodes[i].parent_idx];
	}

	/* Break the layers down into meshes and add those to the model's list. */
	struct HGPLayerHeader *layer_headers = (struct HGPLayerHeader *)OFFSET(model_header->layer_header_offset);

	for (int i = 0; i < model_header->num_layers; i++) {
		/* XXX: Use model configuration to specify layers by quality. */
		if (i != 0 && i != 2)
			continue;

		for (int j = 0; j < 4; j++) {
			if (!layer_headers[i].mesh_header_list_offsets[j])
				continue;

			if (j % 2 == 0) {
				uint32_t *mesh_header_offsets = (uint32_t *)OFFSET(layer_headers[i].mesh_header_list_offsets[j]);

				for (int k = 0; k < model_header->num_meshes; k++)
					this->processMesh(body, mesh_header_offsets[k], model_transforms[k]);
			}
			else
				this->processMesh(body, layer_headers[i].mesh_header_list_offsets[j], static_transform_matrices[0] * model_transforms[0]);
		}
	}

	free(buf);
}
