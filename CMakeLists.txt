cmake_minimum_required(VERSION 3.12)
project(mortar)

set(OpenGL_GL_PREFERENCE GLVND)
SET(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)

set(SRCS
  game/config.cpp
  game/lsw/config.cpp
  render/gl/renderer.cpp
  render/gl/shader.cpp
  resource/character/character.cpp
  resource/character/description.cpp
  resource/providers/lsw/lsw.cpp
  resource/providers/lsw/meshes.cpp
  resource/providers/dds.cpp
  resource/providers/hgp.cpp
  resource/providers/nup.cpp
  resource/actor.cpp
  resource/geom.cpp
  resource/instance.cpp
  resource/joint.cpp
  resource/layer.cpp
  resource/manager.cpp
  resource/material.cpp
  resource/matrix.cpp
  resource/mesh.cpp
  resource/model.cpp
  resource/resource.cpp
  resource/scene.cpp
  resource/texture.cpp
  resource/vertex.cpp
  scene/manager.cpp
  streams/filestream.cpp
  streams/memorystream.cpp
  streams/stream.cpp
  main.cpp
  state.cpp
  )

add_executable(mortar ${SRCS})
target_link_libraries(mortar ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
