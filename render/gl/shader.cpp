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
#include <stdexcept>
#include <vector>

#include "../../log.hpp"
#include "../../resource/shader.hpp"
#include "shader.hpp"

using namespace Mortar::Render::GL;

#define GLSL(src) "#version 150\n" #src

const char *unlitVertexSource = GLSL(
  uniform mat4 projViewMtx;
  uniform mat4 meshTransformMtx;

  uniform vec3 materialColor;
  uniform vec2 colorMultipliers;
  uniform vec2 alphaAnimUV;

  in vec3 position;
  in vec4 color;
  in vec2 texCoord;

  out vec2 fragTexCoord;
  out vec4 fragColor;

  void main()
  {
    fragTexCoord = texCoord;

    vec3 adjustedVertColor = colorMultipliers.x * vec3(color.xyz);
    vec3 adjustedMatColor = colorMultipliers.y * materialColor;

    fragColor = vec4(adjustedVertColor + adjustedMatColor, color.w);

    gl_Position = projViewMtx * meshTransformMtx * vec4(position, 1.0);
  }
);

const GLchar *unlitFragmentSource = GLSL(
  uniform sampler2D materialTex;
  uniform int hasTexture;

  in vec2 fragTexCoord;
  in vec4 fragColor;

  out vec4 outColor;

  void main()
  {
    vec4 texColor = texture(materialTex, fragTexCoord);

    vec4 combinedColor = (texColor * fragColor);

    outColor = mix(fragColor, combinedColor, hasTexture) * 2;
  }
);

const GLchar *skinVertexSource = GLSL(
  uniform mat4 projViewMtx;
  uniform mat4 meshTransformMtx;
	uniform mat4x3 skinTransformMtces[16];

  uniform vec3 materialColor;
  uniform vec2 colorMultipliers;

  in vec3 position;
	in vec2 blendweight;
	in vec3 blendIndices;
  in vec3 normal;
  in vec4 color;
  in vec2 texCoord;

  out vec4 fragColor;
  out vec2 fragTexCoord;

  void main()
  {
    fragTexCoord = texCoord;

		vec3 finalTransformIndices = floor(blendIndices) * 3;

    vec3 transformedNormal = (meshTransformMtx * vec4(normal, 1.0)).xyz;

    vec3 light0Color = max(dot(normalize(vec3(1.0, 0.0, 0.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);
    vec3 light1Color = max(dot(normalize(vec3(0.0, 1.0, 0.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);
    vec3 light2Color = max(dot(normalize(vec3(0.0, 0.0, -1.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);

    fragColor = vec4(materialColor * (light0Color, light1Color, light2Color + vec3(0.4, 0.4, 0.4)), color.w);

    gl_Position = projViewMtx * meshTransformMtx * vec4(position, 1.0);
  }
);

const GLchar *skinFragmentSource = GLSL(
  uniform sampler2D materialTex;
  uniform int hasTexture;

  in vec4 fragColor;
  in vec2 fragTexCoord;

  out vec4 outColor;

  void main()
  {
    vec4 texColor = texture(materialTex, fragTexCoord);

    vec4 combinedTexColor = texColor * fragColor * 2;
    vec4 scaledFragColor = vec4(fragColor.xyz, fragColor.w * 2);

    outColor = mix(scaledFragColor, combinedTexColor, hasTexture);
  }
);

const GLchar *basicVertexSource = GLSL(
  uniform mat4 projViewMtx;
  uniform mat4 meshTransformMtx;

  uniform vec3 materialColor;
  uniform vec2 colorMultipliers;

  in vec3 position;
  in vec3 normal;
  in vec4 color;
  in vec2 texCoord;

  out vec4 fragColor;
  out vec2 fragTexCoord;

  void main()
  {
    fragTexCoord = texCoord;

    vec3 transformedNormal = (meshTransformMtx * vec4(normal, 1.0)).xyz;

    vec3 light0Color = max(dot(normalize(vec3(1.0, 0.0, 0.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);
    vec3 light1Color = max(dot(normalize(vec3(0.0, 1.0, 0.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);
    vec3 light2Color = max(dot(normalize(vec3(0.0, 0.0, -1.0)), transformedNormal), 0) * vec3(1.0, 1.0, 1.0);

    fragColor = vec4(materialColor * (light0Color, light1Color, light2Color + vec3(0.4, 0.4, 0.4)), color.w);

    gl_Position = projViewMtx * meshTransformMtx * vec4(position, 1.0);
  }
);

const GLchar *basicFragmentSource = GLSL(
  uniform sampler2D materialTex;
  uniform int hasTexture;

  in vec4 fragColor;
  in vec2 fragTexCoord;

  out vec4 outColor;

  void main()
  {
    vec4 texColor = texture(materialTex, fragTexCoord);

    vec4 combinedTexColor = texColor * fragColor * 2;
    vec4 scaledFragColor = vec4(fragColor.xyz, fragColor.w * 2);

    outColor = mix(scaledFragColor, combinedTexColor, hasTexture);
  }
);

const GLchar *shaderSources[Mortar::Resource::getShaderCount()][2] = {
  { unlitVertexSource, unlitFragmentSource },
  { skinVertexSource, skinFragmentSource },
  { basicVertexSource, basicFragmentSource },
};

int checkCompileStatus(GLuint shader) {
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    DEBUG("failed to compile shader: %s", infoLog);
    return -1;
  }

  return 0;
}

void ShaderManager::ShaderProgram::shutDown() {
  if (this->program != -1) {
    glDeleteProgram(this->program);
    this->program = -1;
  }

  if (this->vertexShader != -1) {
    glDeleteShader(this->vertexShader);
    this->vertexShader = -1;
  }

  if (this->fragmentShader != -1) {
    glDeleteShader(this->fragmentShader);
    this->fragmentShader = -1;
  }
}

void ShaderManager::ShaderProgram::initialize(const GLchar *vertexShaderSrc, const GLchar *fragmentShaderSrc) {
  /* Compile and link shaders. */
  this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(this->vertexShader, 1, &vertexShaderSrc, NULL);
  glCompileShader(this->vertexShader);
  if (checkCompileStatus(this->vertexShader) == -1) {
    throw std::runtime_error("failed to compile vertex shader");
  }

  this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(this->fragmentShader, 1, &fragmentShaderSrc, NULL);
  glCompileShader(this->fragmentShader);
  if (checkCompileStatus(this->fragmentShader) == -1) {
    throw std::runtime_error("failed to compile vertex shader");
  }

  this->program = glCreateProgram();
  glAttachShader(this->program, vertexShader);
  glAttachShader(this->program, fragmentShader);
  glBindFragDataLocation(this->program, 0, "outColor");
  glLinkProgram(this->program);

  int success;
  glGetProgramiv(this->program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(this->program, 512, NULL, infoLog);
    throw std::runtime_error("failed to link shaders");
  }

  DEBUG("created program %d, successful link: %d", this->program, success);
}

GLuint ShaderManager::ShaderProgram::getShaderProgram() {
  return this->program;
}

ShaderManager::ShaderManager() {
  this->shaderPrograms.resize(Mortar::Resource::getShaderCount());
  for (auto program = this->shaderPrograms.begin(); program != this->shaderPrograms.end(); program++) {
    *program = new ShaderProgram();
  }
}

void ShaderManager::initialize() {
  for (int i = 0; i < this->shaderPrograms.size(); i++) {
    this->shaderPrograms[i]->initialize(shaderSources[i][0], shaderSources[i][1]);
  }
}

void ShaderManager::shutDown() {
  for (auto program = this->shaderPrograms.begin(); program != this->shaderPrograms.end(); program++) {
    (*program)->shutDown();
    delete *program;
  }

  this->shaderPrograms.clear();
}

GLuint ShaderManager::getShaderProgram(Resource::ShaderType shaderType) {
  if (shaderType == Resource::ShaderType::INVALID) {
    throw std::runtime_error("invalid shader type");
  }

  return this->shaderPrograms[static_cast<size_t>(shaderType)]->getShaderProgram();
}
