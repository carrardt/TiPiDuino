cmake_minimum_required (VERSION 2.6)
project(GyroTeapot)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

add_executable(teapot teapot.c texture.c)
target_link_libraries(teapot ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
