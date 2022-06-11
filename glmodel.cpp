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

#include "glmodel.hpp"
#include "log.hpp"

static GLenum getGLPrimitiveType(int prim_type) {
	GLenum gl_prim_type = GL_POINTS;

	switch (prim_type) {
		case 1:
		case 4:
			gl_prim_type = GL_LINES;
			break;
		case 2:
		case 5:
			gl_prim_type = GL_TRIANGLES;
			break;
		case 3:
		case 6:
			gl_prim_type = GL_TRIANGLE_STRIP;
			break;
	}

	return gl_prim_type;
}

GLModel::GLModel(Model model, GLuint shaderProgram) {
	/* Initialize all vertex buffers. */
	this->vertexArrayIds = new GLuint[model.getVertexBufferCount()];
	this->vertexBufferIds = new GLuint[model.getVertexBufferCount()];

	this->vertexBufferCount = model.getVertexBufferCount();

	glGenVertexArrays(this->vertexBufferCount, this->vertexArrayIds);
	glGenBuffers(this->vertexBufferCount, this->vertexBufferIds);

	GLint positionAttr = glGetAttribLocation(shaderProgram, "position");
	GLint colorAttr = glGetAttribLocation(shaderProgram, "color");
	GLint texcoordAttr = glGetAttribLocation(shaderProgram, "texCoord");

	for (int i = 0; i < model.getVertexBufferCount(); i++) {
		glBindVertexArray(this->vertexArrayIds[i]);
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferIds[i]);

		Model::VertexBuffer vertexBuffer = model.getVertexBuffer(i);

		glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size, vertexBuffer.data, GL_STATIC_DRAW);

		glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, vertexBuffer.stride, (GLvoid *)0);
		glEnableVertexAttribArray(positionAttr);

		glVertexAttribPointer(colorAttr, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, vertexBuffer.stride, (GLvoid *)24);
		glEnableVertexAttribArray(colorAttr);

		glVertexAttribPointer(texcoordAttr, 2, GL_FLOAT, GL_FALSE, vertexBuffer.stride, (GLvoid *)28);
		glEnableVertexAttribArray(texcoordAttr);
	}

	DEBUG("model has %d vertex buffers", model.getVertexBufferCount());

	/* Initialize GL texture objects. */
	this->textureIds = new GLuint[model.getTextureCount()];
	this->textureCount = model.getTextureCount();

	glGenTextures(this->textureCount, this->textureIds);

	for (int i = 0; i < model.getTextureCount(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, this->textureIds[i]);

		Texture texture = model.getTexture(i);

		if (texture.compressed)
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, texture.internal_format, texture.width, texture.height, 0, texture.levels[0].size, texture.levels[0].data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	DEBUG("model has %d textures", model.getTextureCount());

	/* Initialize an element buffer for each face. */
	for (int i = 0; i < model.getObjectCount(); i++) {
		Model::Object object = model.getObject(i);

		for (int j = 0; j < object.meshes.size(); j++) {
			Model::Mesh mesh = object.meshes[j];

			for (int k = 0; k < mesh.faces.size(); k++) {
				Model::Face face = mesh.faces[k];
				RenderObject renderObject;

				glGenBuffers(1, &renderObject.elementBuffer);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObject.elementBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * face.num_elements, face.element_buffer, GL_STATIC_DRAW);

				renderObject.vertexArray = this->vertexArrayIds[mesh.vertex_buffer_idx];

				renderObject.primitiveType = getGLPrimitiveType(face.primitive_type);
				renderObject.elementCount = face.num_elements;
				renderObject.transformation = object.transformation;
				renderObject.material = model.getMaterial(mesh.material_idx);

				this->renderObjects.push_back(renderObject);
			}
		}
	}

	DEBUG("model has %d objects for render", model.getObjectCount());
}

GLModel::~GLModel() {
	for (int i = 0; i < this->renderObjects.size(); i++)
		glDeleteBuffers(1, &this->renderObjects[i].elementBuffer);

	glDeleteTextures(this->textureCount, this->textureIds);
	glDeleteBuffers(this->vertexBufferCount, this->vertexBufferIds);
	glDeleteVertexArrays(this->vertexBufferCount, this->vertexArrayIds);
}
