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

#include "mesh.hpp"
#include "shader.hpp"
#include "vertex.hpp"

using namespace Mortar::Resource;

PrimitiveType Surface::getPrimitiveType() const {
  return this->primitiveType;
}

void Surface::setPrimitiveType(PrimitiveType primitiveType) {
  this->primitiveType = primitiveType;
}

const IndexBuffer *Surface::getIndexBuffer() const {
  return this->indexBuffer;
}

void Surface::setIndexBuffer(IndexBuffer *indexBuffer) {
  this->indexBuffer = indexBuffer;
}

unsigned Surface::getSkinTransformCount() const {
  return this->skinTransformCount;
}

void Surface::setSkinTransformCount(unsigned count) {
  this->skinTransformCount = count;
}

const std::vector<ushort>& Surface::getSkinTransformIndices() const {
  return this->skinTransformIndices;
}

void Surface::setSkinTransformIndices(std::vector<ushort>& indices) {
  this->skinTransformIndices = indices;
}

void Mesh::addSurface(Surface *surface) {
  this->surfaces.push_back(surface);
}

const std::vector<Surface *>& Mesh::getSurfaces() const {
  return this->surfaces;
}

const Material *Mesh::getMaterial() const {
  return this->material;
}

void Mesh::setMaterial(Material *material) {
  this->material = material;
}

const VertexBuffer *Mesh::getVertexBuffer() const {
  return this->vertexBuffer;
}

void Mesh::setVertexBuffer(VertexBuffer *vertexBuffer) {
  this->vertexBuffer = vertexBuffer;
}

ShaderType Mesh::getShaderType() const {
  return this->shaderType;
}

void Mesh::setShaderType(ShaderType shaderType) {
  this->shaderType = shaderType;
}

const VertexLayout& Mesh::getVertexLayout() const {
  return this->vertexLayout;
}

void Mesh::setVertexLayout(const VertexLayout &vertexLayout) {
  this->vertexLayout = vertexLayout;
}

unsigned KinematicMesh::getJointIdx() const {
  return this->jointIdx;
}

void KinematicMesh::setJointIdx(unsigned jointIdx) {
  this->jointIdx = jointIdx;
}
