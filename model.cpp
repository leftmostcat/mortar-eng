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

#include <stdlib.h>
#include "model.hpp"

void Model::setVertexBuffers(std::vector<Model::VertexBuffer> vertexBuffers) {
	this->vertexBuffers = vertexBuffers;
}

Model::VertexBuffer Model::getVertexBuffer(int i) {
	return this->vertexBuffers.at(i);
}

int Model::getVertexBufferCount() {
	return this->vertexBuffers.size();
}

void Model::setTextures(std::vector<Texture> textures) {
	this->textures = textures;
}

Texture Model::getTexture(int i) {
	return this->textures.at(i);
}

int Model::getTextureCount() {
	return this->textures.size();
}

void Model::setMaterials(std::vector<Model::Material> materials) {
	this->materials = materials;
}

Model::Material Model::getMaterial(int i) {
	return this->materials.at(i);
}

void Model::setObjects(std::vector<Model::Object> objects) {
	this->objects = objects;

	std::vector<Model::Face>::iterator iter = this->faces.begin();
	for (int i = 0; i < objects.size(); i++) {
		for (int j = 0; j < objects[i].meshes.size(); j++) {
			for (int k = 0; k < objects[i].meshes[j].faces.size(); k++) {
				Face face = objects[i].meshes[j].faces[k];

				face.transform = objects[i].transformation;

				this->faces.push_back(face);
			}
		}
	}
}

Model::Object &Model::getObject(int i) {
	return this->objects[i];
}

int Model::getObjectCount() {
	return this->objects.size();
}

Model::Face &Model::getFace(int i) {
	return this->faces[i];
}

int Model::getFaceCount() {
	return this->faces.size();
}
