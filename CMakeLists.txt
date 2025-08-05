cmake_minimum_required(VERSION 3.10)
project(OpenGLPlayground)

set(CMAKE_CXX_STANDARD 17)

# GLFW
include_directories(external/GLFW/include)

# GLAD
include_directories(external/glad/include)

add_executable(OpenGLPlayground src/main.cpp)

target_link_libraries(OpenGLPlayground glfw glad)

# Link OpenGL
if (WIN32)
    target_link_libraries(OpenGLPlayground opengl32)
endif()
