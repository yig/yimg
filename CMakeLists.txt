cmake_minimum_required(VERSION 2.8.6)


## Compile a static libyimg library.
add_library( yimg STATIC
    YImage.cpp
    )

## A simple program to show how to use YImage.hpp
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIRS})

add_executable(showpng
    showpng.cpp
    )
target_link_libraries(showpng
    yimg
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    )

# Copy the test image
ADD_CUSTOM_COMMAND(
    TARGET showpng
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/sakura.png" "${CMAKE_BINARY_DIR}/sakura.png"
    )
