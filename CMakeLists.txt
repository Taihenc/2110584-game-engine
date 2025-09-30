cmake_minimum_required(VERSION 3.10)
project(OpenGLPlayground)

set(CMAKE_CXX_STANDARD 17)

# Create the executable target first
add_executable(OpenGLPlayground src/main.cpp external/glad/src/glad.c)

# Now configure the target
target_include_directories(OpenGLPlayground PRIVATE
    external/glad/include
    external/KHR/include
    external/GLFW/include
    external/glm/include
    src
)

# Link libraries
target_link_libraries(OpenGLPlayground 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/lib-mingw-w64/libglfw3.a
)

# Link OpenGL and other Windows libraries
if (WIN32)
    target_link_libraries(OpenGLPlayground opengl32 gdi32 user32 kernel32)
endif()