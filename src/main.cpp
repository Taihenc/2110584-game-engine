#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void drawHyperbola(float a = 0.3f, float b = 0.5f, int segments = 100) {
    glColor3f(1.0f, 0.75f, 0.8f);  // Pink color

    // Draw both branches of hyperbola
    for (int branch = 0; branch < 2; branch++) {
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < segments; i++) {
            float t1 = -2.0f + (4.0f * i) / segments;
            float t2 = -2.0f + (4.0f * (i + 1)) / segments;

            float x1 = (branch == 0 ? 1 : -1) * a * cosh(t1);
            float y1 = b * sinh(t1);
            float x2 = (branch == 0 ? 1 : -1) * a * cosh(t2);
            float y2 = b * sinh(t2);

            // Triangle from origin to two consecutive points
            glVertex2f(0.0f, 0.0f);  // Origin
            glVertex2f(x1, y1);      // Point 1
            glVertex2f(x2, y2);      // Point 2
        }
        glEnd();
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "Pink Hyperbola", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawHyperbola();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}