#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <vector>

#include "log.hpp"
#include "shader.hpp"

#define GLSL(src) "#version 150\n" #src

const GLchar *unlitVertexSource = GLSL(
  uniform mat4 projViewMtx;
  uniform mat4 meshTransformMtx;

  uniform vec4 materialColor;
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
    vec3 adjustedMatColor = colorMultipliers.y * vec3(materialColor.xyz);

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

  uniform vec4 materialColor;
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

    fragColor = vec4(materialColor.xyz * (light0Color, light1Color, light2Color + vec3(0.4, 0.4, 0.4)), color.w);

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

  uniform vec4 materialColor;
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

    fragColor = vec4(materialColor.xyz * (light0Color, light1Color, light2Color + vec3(0.4, 0.4, 0.4)), color.w);

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

const GLchar *shaderSources[SHADER_COUNT][2] = {
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

EffectManager::ShaderProgram::ShaderProgram() {
  this->vertexShader = -1;
  this->fragmentShader = -1;
  this->program = -1;
}

EffectManager::ShaderProgram::~ShaderProgram() {
  if (this->program != -1) {
    glDeleteProgram(this->program);
  }

  if (this->vertexShader != -1) {
    glDeleteShader(this->vertexShader);
  }

  if (this->fragmentShader != -1) {
    glDeleteShader(this->fragmentShader);
  }
}

int EffectManager::ShaderProgram::Initialize(const GLchar *vertexShaderSrc, const GLchar *fragmentShaderSrc) {
  /* Compile and link shaders. */
  this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(this->vertexShader, 1, &vertexShaderSrc, NULL);
  glCompileShader(this->vertexShader);
  if (checkCompileStatus(this->vertexShader) == -1) {
    return -1;
  }

  this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(this->fragmentShader, 1, &fragmentShaderSrc, NULL);
  glCompileShader(this->fragmentShader);
  if (checkCompileStatus(this->fragmentShader) == -1) {
    return -1;
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
    DEBUG("failed to link shaders: %s", infoLog);
    return -1;
  }

  DEBUG("created program %d, successful link: %d", this->program, success);

  return 0;
}

GLuint EffectManager::ShaderProgram::GetShaderProgram() {
  return this->program;
}

EffectManager::EffectManager() {
  this->shaderPrograms.resize(SHADER_COUNT);
  std::fill(this->shaderPrograms.begin(), this->shaderPrograms.end(), new ShaderProgram());
}

EffectManager::~EffectManager() {}

int EffectManager::Initialize() {
  for (int i = 0; i < SHADER_COUNT; i++) {
    int status = this->shaderPrograms[i]->Initialize(shaderSources[i][0], shaderSources[i][1]);
    if (status == -1) {
      return -1;
    }
  }

  return 0;
}

GLuint EffectManager::GetShaderProgram(Shader shaderType) {
  return this->shaderPrograms[shaderType]->GetShaderProgram();
}
