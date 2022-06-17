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

#include <vector>

#include "glmodel.hpp"
#include "log.hpp"
#include "shader.hpp"

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

GLModel::GLModel(Model model, EffectManager *effectManager) {
	/* Initialize all vertex buffers. */
	this->vertexBufferIds = new GLuint[model.getVertexBufferCount()];

	this->vertexBufferCount = model.getVertexBufferCount();

	glGenBuffers(this->vertexBufferCount, this->vertexBufferIds);

	for (int i = 0; i < model.getVertexBufferCount(); i++) {
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferIds[i]);

		Model::VertexBuffer vertexBuffer = model.getVertexBuffer(i);

		glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size, vertexBuffer.data, GL_STATIC_DRAW);
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

	/* Initialize an element buffer and vertex array for each face. */
	this->vertexArrayIds = new GLuint[model.getFaceCount()];
	glGenVertexArrays(model.getFaceCount(), this->vertexArrayIds);

	for (int i = 0; i < model.getFaceCount(); i++) {
		Model::Face face = model.getFace(i);
		RenderObject renderObject;

		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferIds[face.vertex_buffer_idx]);
		glBindVertexArray(vertexArrayIds[i]);

		GLuint shaderProgram;

		GLint positionAttr;
		GLint normalAttr;
		GLint colorAttr;
		GLint texcoordAttr;

		switch (face.shaderType) {
			case UNLIT:
			case BASIC:
				shaderProgram = effectManager->GetShaderProgram(UNLIT);

				positionAttr = glGetAttribLocation(shaderProgram, "position");
				normalAttr = glGetAttribLocation(shaderProgram, "normal");
				colorAttr = glGetAttribLocation(shaderProgram, "color");
				texcoordAttr = glGetAttribLocation(shaderProgram, "texCoord");

				glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, face.stride, (GLvoid *)0);
				glEnableVertexAttribArray(positionAttr);

				if (normalAttr != -1) {
					glVertexAttribPointer(normalAttr, 3, GL_FLOAT, GL_TRUE, face.stride, (GLvoid *)12);
					glEnableVertexAttribArray(normalAttr);
				}

				glVertexAttribPointer(colorAttr, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, face.stride, (GLvoid *)24);
				glEnableVertexAttribArray(colorAttr);

				glVertexAttribPointer(texcoordAttr, 2, GL_FLOAT, GL_FALSE, face.stride, (GLvoid *)28);
				glEnableVertexAttribArray(texcoordAttr);
				break;
			case SKIN:
				shaderProgram = effectManager->GetShaderProgram(SKIN);

				positionAttr = glGetAttribLocation(shaderProgram, "position");
				normalAttr = glGetAttribLocation(shaderProgram, "normal");
				colorAttr = glGetAttribLocation(shaderProgram, "color");
				texcoordAttr = glGetAttribLocation(shaderProgram, "texCoord");

				glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, face.stride, (GLvoid *)0);
				glEnableVertexAttribArray(positionAttr);

				glVertexAttribPointer(normalAttr, 3, GL_FLOAT, GL_TRUE, face.stride, (GLvoid *)32);
				glEnableVertexAttribArray(normalAttr);

				glVertexAttribPointer(colorAttr, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, face.stride, (GLvoid *)44);
				glEnableVertexAttribArray(colorAttr);

				glVertexAttribPointer(texcoordAttr, 2, GL_FLOAT, GL_FALSE, face.stride, (GLvoid *)48);
				glEnableVertexAttribArray(texcoordAttr);
				break;
			default:
				DEBUG("unknown shader type %d on face %d", face.shaderType, i);
				continue;
		}

		renderObject.shaderType = face.shaderType;

		glGenBuffers(1, &renderObject.elementBuffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObject.elementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * face.num_elements, face.element_buffer, GL_STATIC_DRAW);

		renderObject.vertexArray = this->vertexArrayIds[i];

		renderObject.primitiveType = getGLPrimitiveType(face.primitive_type);
		renderObject.elementCount = face.num_elements;
		renderObject.transformation = face.transform;
		renderObject.material = model.getMaterial(face.materialIdx);

		if (renderObject.material.flags & Model::Material::ENABLE_ALPHA_BLEND) {
			DEBUG("alpha ref is %f", (float)((renderObject.material.rawFlags >> 0x17 & 0xff) << 1) / 255.0);
			this->alphaRenderObjects.push_back(renderObject);
		} else {
			this->renderObjects.push_back(renderObject);
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
