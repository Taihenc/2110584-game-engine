# OpenGL Playground

A minimal C++ OpenGL playground using GLFW and GLAD.

## Setup

1. Download and place GLFW and GLAD source code in the `external/` folder:
   - [GLFW](https://github.com/glfw/glfw) → `external/glfw`
   - [GLAD (C/C++ OpenGL 3.3 Core)](https://github.com/Dav1dde/glad) → `external/glad`
2. Build with CMake:
   ```powershell
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
3. Run the executable from the `build` folder.

## Requirements

- CMake 3.10+
- A C++17 compiler

## Notes

- The playground opens a window and clears the screen with a color.
- Extend `src/main.cpp` for your OpenGL experiments.
