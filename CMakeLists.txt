cmake_minimum_required(VERSION 3.0)
project(mortar)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)

set(SRCS
	main.cpp
	dds.cpp
	filestream.cpp
	glmodel.cpp
	hgp.cpp
	lsw.cpp
	matrix.cpp
	memorystream.cpp
	model.cpp
	nup.cpp
	stream.cpp
	texture.cpp
	)

add_executable(mortar ${SRCS})
target_link_libraries(mortar ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
