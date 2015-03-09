cmake_minimum_required(VERSION 2.8)
project(mortar)

find_package(OpenGL)
find_package(PkgConfig)

pkg_check_modules(GLFW3 REQUIRED glfw3)

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
target_link_libraries(mortar ${OPENGL_LIBRARIES} ${GLFW3_LIBRARIES} ${GLFW3_STATIC_LIBRARIES})
