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

#ifndef MORTAR_MESH_H
#define MORTAR_MESH_H

#include <stdint.h>
#include <vector>

#include "../resource.hpp"
#include "joint.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "vertex.hpp"

namespace Mortar::Resource {
  enum class PrimitiveType {
    LINE_LIST,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
  };

  class Surface : public Resource {
    public:
      Surface(ResourceHandle handle)
        : Resource { handle } {};

      PrimitiveType getPrimitiveType() const;
      void setPrimitiveType(PrimitiveType primitiveType);

      const IndexBuffer *getIndexBuffer() const;
      void setIndexBuffer(IndexBuffer *indexBuffer);

      unsigned getSkinTransformCount() const;
      void setSkinTransformCount(unsigned count);

      const std::vector<ushort>& getSkinTransformIndices() const;
      void setSkinTransformIndices(std::vector<ushort>& indices);

    private:
      PrimitiveType primitiveType;

      IndexBuffer *indexBuffer;

      unsigned skinTransformCount;
      std::vector<ushort> skinTransformIndices;
  };

  class Mesh : public Resource {
    public:
      Mesh(ResourceHandle handle)
        : Resource { handle },
          shaderType { ShaderType::INVALID },
          vertexLayout { VertexLayout::EMPTY } {};

      void addSurface(Surface *surface);
      const std::vector<Surface *>& getSurfaces() const;

      const Material *getMaterial() const;
      void setMaterial(Material *material);

      const VertexBuffer *getVertexBuffer() const;
      void setVertexBuffer(VertexBuffer *vertexBuffer);

      ShaderType getShaderType() const;
      void setShaderType(ShaderType shaderType);

      const VertexLayout& getVertexLayout() const;
      void setVertexLayout(const VertexLayout& vertexLayout);

    private:
      std::vector<Surface *> surfaces;

      Material *material;
      VertexBuffer *vertexBuffer;

      ShaderType shaderType;
      VertexLayout vertexLayout;
  };

  class KinematicMesh : public Resource {
    public:
      KinematicMesh(ResourceHandle handle)
        : Resource { handle } {};

      const Mesh *getMesh() const;
      void setMesh(Mesh *);

      unsigned getJointIdx() const;
      void setJointIdx(unsigned jointIdx);

    private:
      unsigned jointIdx;
      const Mesh *mesh;
  };
}

#endif
