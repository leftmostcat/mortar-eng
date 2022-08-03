cmake_minimum_required(VERSION 3.12)
project(mortar)

set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
find_package(tsl-sparse-map REQUIRED)

set(SRCS
  anim/anim.cpp
  camera.cpp
  clock.cpp
  display.cpp
  game/lsw/game.cpp
  game/lsw/loaders/character.cpp
  game/lsw/loaders/scene.cpp
  game/lsw/readers/anim.cpp
  game/lsw/readers/dds.cpp
  game/lsw/readers/hgp.cpp
  game/lsw/readers/common/common.cpp
  game/lsw/readers/common/meshes.cpp
  game/lsw/readers/nup.cpp
  math/matrix.cpp
  render/gl/renderer.cpp
  render/gl/shader.cpp
  resource/manager.cpp
  resource/resource.cpp
  resource/types/actor.cpp
  resource/types/anim.cpp
  resource/types/character.cpp
  resource/types/geom.cpp
  resource/types/instance.cpp
  resource/types/joint.cpp
  resource/types/layer.cpp
  resource/types/material.cpp
  resource/types/mesh.cpp
  resource/types/model.cpp
  resource/types/scene.cpp
  resource/types/spline.cpp
  resource/types/texture.cpp
  resource/types/vertex.cpp
  scene/manager.cpp
  streams/filestream.cpp
  streams/memorystream.cpp
  streams/stream.cpp
  main.cpp
  state.cpp
  )

add_executable(mortar ${SRCS})
target_link_libraries(mortar ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
