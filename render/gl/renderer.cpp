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

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdexcept>
#include <tsl/sparse_map.h>
#include <vector>

#include "../../log.hpp"
#include "../../state.hpp"
#include "renderer.hpp"
#include "shader.hpp"

#define WIDTH 800
#define HEIGHT 600

using namespace Mortar::Render::GL;

void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
  DEBUG("gl error: %s", message);

  std::terminate();
}

const tsl::sparse_map<Mortar::Resource::PrimitiveType, GLenum> primitiveTypeMap = {
  { Mortar::Resource::PrimitiveType::LINE_LIST,      GL_LINES          },
  { Mortar::Resource::PrimitiveType::TRIANGLE_LIST,  GL_TRIANGLES      },
  { Mortar::Resource::PrimitiveType::TRIANGLE_STRIP, GL_TRIANGLE_STRIP },
};

static inline GLenum getGLPrimitiveType(Mortar::Resource::PrimitiveType mortarType) {
  if (!primitiveTypeMap.contains(mortarType)) {
    throw std::runtime_error("unrecognized primitive type");
  }

  return primitiveTypeMap.at(mortarType);
}

const tsl::sparse_map<Mortar::Resource::VertexUsage, std::string> vertexUsageMap = {
  { Mortar::Resource::VertexUsage::BLEND_INDICES, "blendIndices" },
  { Mortar::Resource::VertexUsage::BLEND_WEIGHTS, "blendWeights" },
  { Mortar::Resource::VertexUsage::COLOR,         "color"        },
  { Mortar::Resource::VertexUsage::NORMAL,        "normal"       },
  { Mortar::Resource::VertexUsage::POSITION,      "position"     },
  { Mortar::Resource::VertexUsage::TEX_COORD,     "texCoord"     },
};

static inline const char *getVertexPropertyParamName(Mortar::Resource::VertexUsage vertexUsage) {
  if (!vertexUsageMap.contains(vertexUsage)) {
    throw std::runtime_error("unrecognized vertex usage");
  }

  return vertexUsageMap.at(vertexUsage).c_str();
}

struct GLVertexPropertyType {
  GLenum type;
  GLint size;
};

const tsl::sparse_map<Mortar::Resource::VertexDataType, struct GLVertexPropertyType> vertexDataTypeMap = {
  {
    Mortar::Resource::VertexDataType::D3DCOLOR,
    { GL_UNSIGNED_BYTE, GL_BGRA }
  },
  {
    Mortar::Resource::VertexDataType::VEC2,
    { GL_FLOAT, 2 }
  },
  {
    Mortar::Resource::VertexDataType::VEC3,
    { GL_FLOAT, 3 }
  },
};

static inline const struct GLVertexPropertyType& getVertexPropertyType(Mortar::Resource::VertexDataType vertexDataType) {
  if (!vertexDataTypeMap.contains(vertexDataType)) {
    throw std::runtime_error("unrecognized vertex data type");
  }

  return vertexDataTypeMap.at(vertexDataType);
}

void Renderer::initialize() {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GLContext context = SDL_GL_CreateContext(State::getDisplayManager().getWindow());

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
  glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
  glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);

  this->shaderManager.initialize();

  this->isInitialized = true;
}

void Renderer::shutDown() {
  this->shaderManager.shutDown();

  GLuint *textureIds = new GLuint[this->textureIds.size()];
  GLuint *textureIdPtr = textureIds;
  for (auto texture = this->textureIds.begin(); texture != this->textureIds.end(); texture++, textureIdPtr++) {
    *textureIdPtr = texture->second;
  }
  glDeleteTextures(this->textureIds.size(), textureIds);
  delete[] textureIds;

  GLuint *vertexArrayIds = new GLuint[this->vertexArrayIds.size()];
  GLuint *vertexArrayIdPtr = vertexArrayIds;
  for (auto vertexArray = this->vertexArrayIds.begin(); vertexArray != this->vertexArrayIds.end(); vertexArray++, vertexArrayIdPtr++) {
    *vertexArrayIdPtr = vertexArray->second;
  }
  glDeleteVertexArrays(this->vertexArrayIds.size(), vertexArrayIds);
  delete[] vertexArrayIds;

  GLuint *vertexBufferIds = new GLuint[this->vertexBufferIds.size()];
  GLuint *vertexBufferIdPtr = vertexBufferIds;
  for (auto vertexBuffer = this->vertexBufferIds.begin(); vertexBuffer != this->vertexBufferIds.end(); vertexBuffer++, vertexBufferIdPtr++) {
    *vertexBufferIdPtr = vertexBuffer->second;
  }
  glDeleteBuffers(this->vertexBufferIds.size(), vertexBufferIds);
  delete[] vertexBufferIds;

  GLuint *elementBufferIds = new GLuint[this->elementBufferIds.size()];
  GLuint *elementBufferIdPtr = elementBufferIds;
  for (auto elementBuffer = this->elementBufferIds.begin(); elementBuffer != this->elementBufferIds.end(); elementBuffer++, elementBufferIdPtr++) {
    *elementBufferIdPtr = elementBuffer->second;
  }
  glDeleteBuffers(this->elementBufferIds.size(), elementBufferIds);
  delete[] elementBufferIds;
}

void Renderer::registerMeshes(const std::vector<Resource::Mesh *>& meshes) {
  /* Initialize a vertex array for each mesh. */
  GLuint *vertexArrayIds = new GLuint[meshes.size()];
  GLuint *vertexArrayIdPtr = vertexArrayIds;
  glGenVertexArrays(meshes.size(), vertexArrayIds);

  for (auto mesh = meshes.begin(); mesh != meshes.end(); mesh++, vertexArrayIdPtr++) {
    GLuint vertexBufferId = this->vertexBufferIds.at((*mesh)->getVertexBuffer()->getHandle());
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    glBindVertexArray(*vertexArrayIdPtr);
    this->vertexArrayIds[(*mesh)->getHandle()] = *vertexArrayIdPtr;

    Resource::ShaderType shaderType = (*mesh)->getShaderType();
    GLuint shaderProgram = this->shaderManager.getShaderProgram(shaderType);

    const Resource::VertexLayout& vertexLayout = (*mesh)->getVertexLayout();

    unsigned stride = vertexLayout.getStride();
    const std::vector<Resource::VertexLayout::VertexProperty>& vertexProperties = vertexLayout.getProperties();
    for (auto property = vertexProperties.begin(); property != vertexProperties.end(); property++) {
      const char *attribName = getVertexPropertyParamName(property->getUsage());
      GLint attr = glGetAttribLocation(shaderProgram, attribName);
      if (attr == -1) {
        continue;
      }

      const struct GLVertexPropertyType glType = getVertexPropertyType(property->getDataType());
      glVertexAttribPointer(attr, glType.size, glType.type, GL_TRUE, stride, (GLvoid *)property->getOffset());
      glEnableVertexAttribArray(attr);
    }

    const std::vector<Resource::Surface *>& surfaces = (*mesh)->getSurfaces();

    GLuint *elementBufferIds = new GLuint[surfaces.size()];
    GLuint *elementBufferIdPtr = elementBufferIds;
    glGenBuffers(surfaces.size(), elementBufferIds);

    for (auto surface = surfaces.begin(); surface != surfaces.end(); surface++, elementBufferIdPtr++) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *elementBufferIdPtr);
      this->elementBufferIds[(*surface)->getHandle()] = *elementBufferIdPtr;

      const Resource::IndexBuffer *indexBuffer = (*surface)->getIndexBuffer();

      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indexBuffer->getCount(), indexBuffer->getData(), GL_STATIC_DRAW);
    }

    delete[] elementBufferIds;
  }

  delete[] vertexArrayIds;
}

void Renderer::registerTextures(const std::vector<Resource::Texture *> &textures) {
  GLuint *textureIds = new GLuint[textures.size()];
  GLuint *textureIdPtr = textureIds;
  glGenTextures(textures.size(), textureIds);

  size_t start = this->textureSamplers.size();

  for (int i = start; i < start + textures.size(); i++, textureIdPtr++) {
    Resource::Texture *texture = textures.at(i - start);

    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, *textureIdPtr);

    this->textureSamplers[texture->getHandle()] = i;
    this->textureIds[texture->getHandle()] = *textureIdPtr;

    const std::vector<Resource::Texture::Level *>& levels = texture->getLevels();
    for (auto level = levels.begin(); level != levels.end(); level++) {
      GLsizei width = texture->getWidth() >> (*level)->getLevel();
      GLsizei height = texture->getHeight() >> (*level)->getLevel();

      if (!texture->getIsCompressed()) {
        throw std::runtime_error("not expecting uncompressed data");
      }

      glCompressedTexImage2D(GL_TEXTURE_2D, (*level)->getLevel(), texture->getInternalFormat(), width, height, 0, (*level)->getSize(), (*level)->getData());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  delete[] textureIds;
}

void Renderer::registerVertexBuffers(const std::vector<Resource::VertexBuffer *> &vertexBuffers) {
  GLuint *vertexBufferIds = new GLuint[vertexBuffers.size()];
  GLuint *vertexBufferIdPtr = vertexBufferIds;
  glGenBuffers(vertexBuffers.size(), vertexBufferIds);

  /* Initialize all vertex buffers. */
  for (auto vertexBuffer = vertexBuffers.begin(); vertexBuffer != vertexBuffers.end(); vertexBuffer++, vertexBufferIdPtr++) {
    glBindBuffer(GL_ARRAY_BUFFER, *vertexBufferIdPtr);
    this->vertexBufferIds[(*vertexBuffer)->getHandle()] = *vertexBufferIdPtr;

    glBufferData(GL_ARRAY_BUFFER, (*vertexBuffer)->getSize(), (*vertexBuffer)->getData(), GL_STATIC_DRAW);
  }

  delete[] vertexBufferIds;
}

void Renderer::renderGeometry(const std::list<const Resource::GeomObject *>& geometry) {
  if (!this->isInitialized) {
    DEBUG("renderer not initialized");
  }

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  /* Initialize transformation matrices. */
  const Math::Matrix& proj = State::getDisplayManager().getPerspectiveTransform();
  const Math::Matrix& view = State::getCamera().getViewTransform();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (geometry.empty()) {
    return;
  }

  Math::Matrix projViewMtx = view * d3dTransform * proj;

  for (auto geom : geometry) {
    const Resource::Mesh *mesh = geom->mesh;

    GLuint shaderProgram = this->shaderManager.getShaderProgram(mesh->getShaderType());
    glUseProgram(shaderProgram);

    // /* Pull out attribute and uniform locations. */
    GLint tex_unif = glGetUniformLocation(shaderProgram, "materialTex");
    GLint has_tex_unif = glGetUniformLocation(shaderProgram, "hasTexture");

    GLint projViewMtxUnif = glGetUniformLocation(shaderProgram, "projViewMtx");
    GLint worldTransformUnif = glGetUniformLocation(shaderProgram, "meshTransformMtx");

    GLint color_unif = glGetUniformLocation(shaderProgram, "materialColor");
    GLint multipliers_unif = glGetUniformLocation(shaderProgram, "colorMultipliers");
    GLint skinMtcesUnif = glGetUniformLocation(shaderProgram, "skinTransformMtces");

    // if (renderObject.shaderType == UNLIT) {
    //   glUniform2fv(alphaAnimUVUnif, 1, renderObject.material.alphaAnimUV);
    // }

    glUniformMatrix4fv(projViewMtxUnif, 1, GL_FALSE, projViewMtx.f);

    const Resource::Material *material = mesh->getMaterial();

    float adjustedColor[3];
    memcpy(adjustedColor, material->getColor(), 3 * sizeof(float));

    /* Ensure that fragment colors come from the right place. */
    const Resource::Texture *texture = material->getTexture();
    if (texture) {
      GLuint sampler = this->textureSamplers.at(texture->getHandle());

      glUniform1i(tex_unif, sampler);
      glUniform1i(has_tex_unif, 1);

      for (int i = 0; i < 3; i++) {
        adjustedColor[i] *= 0.5f;
      }
    } else {
      glUniform1i(has_tex_unif, 0);
    }

    if (material->isAlphaBlended()) {
      glEnable(GL_BLEND);
      // glEnable(GL_ALPHA_TEST);
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // glAlphaFunc(GL_GEQUAL, (float)((renderObject.material.rawFlags >> 0x17 & 0xff) << 1) / 255.0);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
      glDepthMask(GL_FALSE);
    }

    // float colorMultipliers[2] = {1.0f, 1.0f};
    // if (renderObject.material.flags & Model::Material::USE_VERTEX_COLOR) {
    //   colorMultipliers[0] = 1.0f;
    //   colorMultipliers[1] = 0.0f;
    // }

    /* Set per-mesh material color and transformation matrix. */
    glUniform3fv(color_unif, 1, adjustedColor);

    if (worldTransformUnif != -1) {
      glUniformMatrix4fv(worldTransformUnif, 1, GL_FALSE, geom->worldTransform.f);
    }

    Resource::ResourceHandle meshHandle = mesh->getHandle();

    GLuint vertexArrayId = this->vertexArrayIds.at(meshHandle);
    glBindVertexArray(vertexArrayId);

    std::vector<Math::Matrix> skinTransforms = geom->skinTransforms;

    const std::vector<Resource::Surface *>& surfaces = mesh->getSurfaces();
    for (auto surface = surfaces.begin(); surface != surfaces.end(); surface++) {
      if (skinMtcesUnif != -1) {
        const std::vector<ushort>& indices = (*surface)->getSkinTransformIndices();
        unsigned count = (*surface)->getSkinTransformCount();

        assert(count <= 16);

        float floats[256];
        float *floatPtr = floats;
        for (int i = 0; i < count; i++, floatPtr += 16) {
          if (State::printNextFrame && (*surface)->getIndexBuffer()->getCount() == 30) {
            DEBUG("index at %d is %d", i, indices.at(i));
            DEBUG("base 0x%lx, 0x%lx", (unsigned long)floats, (unsigned long)floatPtr);
          }

          const float *transform = skinTransforms.at(indices.at(i)).f;
          memcpy(floatPtr, transform, 16 * sizeof(float));
        }

        glUniformMatrix4fv(skinMtcesUnif, count, GL_TRUE, floats);
      }

      Resource::ResourceHandle surfaceHandle = (*surface)->getHandle();

      GLuint elementBufferId = this->elementBufferIds.at(surfaceHandle);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);

      GLenum glPrimitiveType = getGLPrimitiveType((*surface)->getPrimitiveType());
      glDrawElements(glPrimitiveType, (*surface)->getIndexBuffer()->getCount(), GL_UNSIGNED_SHORT, 0);
    }

    if (material->isAlphaBlended()) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
      // glDisable(GL_ALPHA_TEST);
    }
  }

  SDL_GL_SwapWindow(State::getDisplayManager().getWindow());
}
