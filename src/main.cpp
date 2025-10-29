#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <learnopengl/shader.h>

#include <cmath>    // for atan2, asin, abs
#include <cstdlib>  // for rand()
#include <ctime>    // for time()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iomanip>  // for std::setprecision
#include <iostream>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "camera.h"
#include "tiny_gltf.h"

// Simple mesh structure for GLTF
struct GLTFMesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int), indices.data(),
                     GL_STATIC_DRAW);

        // Position attribute (assuming 3 floats per vertex)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

// Simple airplane model loader using TinyGLTF
class AirplaneModel {
   public:
    std::vector<GLTFMesh> meshes;
    bool loaded = false;

    bool loadModel(const std::string& path) {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

        if (!warn.empty()) {
            std::cout << "GLTF Warning: " << warn << std::endl;
        }

        if (!err.empty()) {
            std::cout << "GLTF Error: " << err << std::endl;
        }

        if (!ret) {
            std::cout << "Failed to load GLTF: " << path << std::endl;
            return false;
        }

        std::cout << "Successfully loaded GLTF with " << model.meshes.size()
                  << " meshes" << std::endl;

        // Process all meshes
        for (const auto& mesh : model.meshes) {
            for (const auto& primitive : mesh.primitives) {
                GLTFMesh gltfMesh;

                // Get position accessor
                if (primitive.attributes.find("POSITION") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor& posAccessor =
                        model.accessors[primitive.attributes.at("POSITION")];
                    const tinygltf::BufferView& posView =
                        model.bufferViews[posAccessor.bufferView];
                    const tinygltf::Buffer& posBuffer =
                        model.buffers[posView.buffer];

                    const float* positions = reinterpret_cast<const float*>(
                        &posBuffer.data[posView.byteOffset +
                                        posAccessor.byteOffset]);

                    // Build vertex data (just positions for now)
                    for (size_t i = 0; i < posAccessor.count; ++i) {
                        gltfMesh.vertices.push_back(positions[i * 3 + 0]);
                        gltfMesh.vertices.push_back(positions[i * 3 + 1]);
                        gltfMesh.vertices.push_back(positions[i * 3 + 2]);
                    }
                }

                // Get indices
                if (primitive.indices >= 0) {
                    const tinygltf::Accessor& indexAccessor =
                        model.accessors[primitive.indices];
                    const tinygltf::BufferView& indexView =
                        model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer& indexBuffer =
                        model.buffers[indexView.buffer];

                    if (indexAccessor.componentType ==
                        TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const uint16_t* indices =
                            reinterpret_cast<const uint16_t*>(
                                &indexBuffer.data[indexView.byteOffset +
                                                  indexAccessor.byteOffset]);
                        for (size_t i = 0; i < indexAccessor.count; ++i) {
                            gltfMesh.indices.push_back(
                                static_cast<unsigned int>(indices[i]));
                        }
                    } else if (indexAccessor.componentType ==
                               TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const uint32_t* indices =
                            reinterpret_cast<const uint32_t*>(
                                &indexBuffer.data[indexView.byteOffset +
                                                  indexAccessor.byteOffset]);
                        for (size_t i = 0; i < indexAccessor.count; ++i) {
                            gltfMesh.indices.push_back(indices[i]);
                        }
                    }
                }

                gltfMesh.setupMesh();
                meshes.push_back(gltfMesh);
            }
        }

        loaded = true;
        return true;
    }

    void draw() {
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }
};

// Simple airplane model using multiple cubes
void drawAirplane(unsigned int cubeVAO, Shader& shader, glm::vec3 position,
                  glm::vec3 rotation) {
    glm::mat4 baseModel = glm::mat4(1.0f);
    baseModel = glm::translate(baseModel, position);
    baseModel = glm::rotate(baseModel, glm::radians(rotation.y),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    baseModel = glm::rotate(baseModel, glm::radians(rotation.x),
                            glm::vec3(1.0f, 0.0f, 0.0f));
    baseModel = glm::rotate(baseModel, glm::radians(rotation.z),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    // Main body (fuselage)
    glm::mat4 model = baseModel;
    model = glm::scale(model, glm::vec3(0.2f, 0.15f, 1.0f));
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(0.9f, 0.9f, 0.9f));  // Brighter fuselage
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Wings
    model = baseModel;
    model = glm::scale(model, glm::vec3(1.5f, 0.05f, 0.3f));
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(0.85f, 0.85f, 0.85f));  // Brighter wings
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Tail
    model = baseModel;
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.6f));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.2f));
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(0.95f, 0.95f, 0.95f));  // Brighter tail
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Propeller
    model = baseModel;
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.6f));
    model = glm::scale(model, glm::vec3(0.02f, 0.6f, 0.02f));
    shader.setMat4("model", model);
    shader.setVec3("color",
                   glm::vec3(0.6f, 0.6f, 0.6f));  // Much brighter propeller
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
}

// Screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera - positioned behind the plane at the start of the lane
Camera camera(glm::vec3(0.0f, 3.0f,
                        -10.0f));  // Behind plane, slightly elevated
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Airplane position and rotation
glm::vec3 airplanePos(0.0f, 0.0f, 0.0f);
glm::vec3 airplaneRotation(0.0f, 0.0f,
                           0.0f);  // Face forward along the lane (0 degrees)
float airplaneSpeed = 5.0f;

// Lane-based game system
const float LANE_WIDTH = 3.0f;
const float LANE_LEFT = -LANE_WIDTH;  // Left lane at X = -3
const float LANE_CENTER = 0.0f;       // Center lane at X = 0
const float LANE_RIGHT = LANE_WIDTH;  // Right lane at X = 3
int currentLane = 1;           // 0=left, 1=center, 2=right (start in center)
float targetX = LANE_CENTER;   // Target X position for smooth movement
float laneChangeSpeed = 8.0f;  // Speed of lane switching

// Lane-based obstacle system
struct Obstacle {
    glm::vec3 position;
    int lane;  // 0=left, 1=center, 2=right
    bool active;
};

std::vector<Obstacle> obstacles;
float obstacleSpawnDistance = 20.0f;  // How far ahead to spawn obstacles
float obstacleSpacing = 8.0f;         // Distance between obstacle sets
int maxActiveObstacles = 10;          // Maximum obstacles on screen

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawCube(unsigned int VAO, Shader& shader, glm::vec3 position,
              glm::vec3 color);
void spawnObstacle();
void updateObstacles();
void checkCollisions();

int main() {
    // Initialize random seed
    srand(static_cast<unsigned int>(time(0)));

    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Airplane Game", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD initialization
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Set initial camera orientation to look forward along the lane
    camera.Yaw = 90.0f;     // Look forward (towards positive Z)
    camera.Pitch = -10.0f;  // Slightly downward to see the plane
    // Update camera vectors with new yaw/pitch values
    camera.ProcessMouseMovement(0,
                                0);  // This will trigger updateCameraVectors()

    // Shaders
    Shader cubeShader("shaders/cube.vs", "shaders/cube.fs");
    Shader modelShader("shaders/model.vs", "shaders/model.fs");

    AirplaneModel airplane;
    airplane.loadModel("assets/airplane/scene.gltf");

    // Cube VAO for targets
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        checkCollisions();

        // Debug output - Only print when values change significantly
        static glm::vec3 lastAirplanePos = airplanePos;
        static glm::vec3 lastAirplaneRotation = airplaneRotation;
        static glm::vec3 lastCameraPos = camera.Position;
        static float lastCameraYaw = camera.Yaw;
        static float lastCameraPitch = camera.Pitch;

        // Check for significant changes (threshold to avoid tiny floating point
        // differences)
        bool airplanePosChanged =
            glm::length(airplanePos - lastAirplanePos) > 0.1f;
        bool airplaneRotChanged =
            abs(airplaneRotation.y - lastAirplaneRotation.y) > 1.0f;
        bool cameraPosChanged =
            glm::length(camera.Position - lastCameraPos) > 0.1f;
        bool cameraRotChanged = abs(camera.Yaw - lastCameraYaw) > 1.0f ||
                                abs(camera.Pitch - lastCameraPitch) > 1.0f;

        if (airplanePosChanged || airplaneRotChanged || cameraPosChanged ||
            cameraRotChanged) {
            std::cout << "\n=== STATE CHANGED ===" << std::endl;

            if (airplanePosChanged) {
                std::cout << "AIRPLANE Position: (" << std::fixed
                          << std::setprecision(2) << airplanePos.x << ", "
                          << airplanePos.y << ", " << airplanePos.z << ")"
                          << std::endl;
            }

            if (airplaneRotChanged) {
                std::cout << "AIRPLANE Rotation Y: " << std::fixed
                          << std::setprecision(1) << airplaneRotation.y << "°"
                          << std::endl;

                // Show forward direction when rotation changes
                glm::vec3 forward = glm::vec3(
                    sin(glm::radians(airplaneRotation.y + 180.0f)), 0.0f,
                    -cos(glm::radians(airplaneRotation.y + 180.0f)));
                std::cout << "AIRPLANE Forward: (" << std::fixed
                          << std::setprecision(2) << forward.x << ", "
                          << forward.y << ", " << forward.z << ")" << std::endl;
            }

            if (cameraPosChanged) {
                std::cout << "CAMERA Position: (" << std::fixed
                          << std::setprecision(2) << camera.Position.x << ", "
                          << camera.Position.y << ", " << camera.Position.z
                          << ")" << std::endl;
            }

            if (cameraRotChanged) {
                std::cout << "CAMERA Rotation: Yaw " << std::fixed
                          << std::setprecision(1) << camera.Yaw << "°, Pitch "
                          << camera.Pitch << "°" << std::endl;
            }

            // Always show distance when something changes
            float distance = glm::length(camera.Position - airplanePos);
            std::cout << "Distance: " << std::fixed << std::setprecision(1)
                      << distance << " units" << std::endl;
            std::cout << "==================\n" << std::endl;

            // Update last known values
            lastAirplanePos = airplanePos;
            lastAirplaneRotation = airplaneRotation;
            lastCameraPos = camera.Position;
            lastCameraYaw = camera.Yaw;
            lastCameraPitch = camera.Pitch;
        }

        glClearColor(0.7f, 0.8f, 0.9f,
                     1.0f);  // Much brighter sky-like background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Render airplane
        if (airplane.loaded) {
            // Render the actual GLTF airplane model with model shader
            modelShader.use();
            modelShader.setMat4("projection", projection);
            modelShader.setMat4("view", view);
            modelShader.setBool(
                "hasTexture",
                false);  // Will be updated when we add texture loading

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, airplanePos);
            model =
                glm::rotate(model, glm::radians(airplaneRotation.y),
                            glm::vec3(0.0f, 1.0f,
                                      0.0f));  // Normal rotation without offset
            model = glm::rotate(model, glm::radians(airplaneRotation.x),
                                glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(
                model, glm::radians(airplaneRotation.z),
                glm::vec3(0.0f, 0.0f,
                          1.0f));  // Remove 180° Z rotation to fix upside-down
            model = glm::scale(model, glm::vec3(0.5f));  // Scale down the model
            modelShader.setMat4("model", model);

            airplane.draw();
        } else {
            // Fallback to simple cube airplane if GLTF failed to load
            cubeShader.use();
            cubeShader.setMat4("projection", projection);
            cubeShader.setMat4("view", view);
            drawAirplane(cubeVAO, cubeShader, airplanePos, airplaneRotation);
        }

        // Update and render obstacles
        updateObstacles();

        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);

        // Render obstacles (red cubes)
        for (const auto& obstacle : obstacles) {
            if (obstacle.active) {
                drawCube(
                    cubeVAO, cubeShader, obstacle.position,
                    glm::vec3(1.0f, 0.2f, 0.2f));  // Red color for obstacles
            }
        }

        // Optional: Render lane markers for visual reference
        static bool showLaneMarkers = true;
        if (showLaneMarkers) {
            // Draw small markers to show lane positions
            for (int i = 0; i < 3; i++) {
                float laneX = (i == 0)   ? LANE_LEFT
                              : (i == 1) ? LANE_CENTER
                                         : LANE_RIGHT;
                glm::vec3 markerPos =
                    glm::vec3(laneX, -0.5f, airplanePos.z + 5.0f);
                drawCube(cubeVAO, cubeShader, markerPos,
                         glm::vec3(0.3f, 0.3f, 0.3f));  // Gray lane markers
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Debug key - Press P to print current state
    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
        pKeyPressed = true;
        std::cout << "\n=== MANUAL DEBUG (P key pressed) ===" << std::endl;
        std::cout << "AIRPLANE:" << std::endl;
        std::cout << "  Position: (" << airplanePos.x << ", " << airplanePos.y
                  << ", " << airplanePos.z << ")" << std::endl;
        std::cout << "  Current Lane: " << currentLane
                  << " (0=left, 1=center, 2=right)" << std::endl;
        std::cout << "  Target X: " << targetX << std::endl;
        std::cout << "CAMERA:" << std::endl;
        std::cout << "  Position: (" << camera.Position.x << ", "
                  << camera.Position.y << ", " << camera.Position.z << ")"
                  << std::endl;
        std::cout << "  Yaw: " << camera.Yaw << "°, Pitch: " << camera.Pitch
                  << "°" << std::endl;
        std::cout << "=================================\n" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pKeyPressed = false;
    }

    // Lane-based airplane movement
    bool airplaneMoved = false;

    // W key - forward movement (plane always faces forward along positive Z
    // axis)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 forward =
            glm::vec3(0.0f, 0.0f, 1.0f);  // Always move in positive Z direction
        airplanePos += forward * airplaneSpeed * deltaTime;
        airplaneMoved = true;
    }

    // Lane switching with A/D keys
    static bool aKeyPressed = false;
    static bool dKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !aKeyPressed) {
        aKeyPressed = true;
        // Move to left lane
        if (currentLane > 0) {
            currentLane--;
            switch (currentLane) {
                case 0:
                    targetX = LANE_LEFT;
                    break;
                case 1:
                    targetX = LANE_CENTER;
                    break;
                case 2:
                    targetX = LANE_RIGHT;
                    break;
            }
            std::cout << "Switching to lane " << currentLane
                      << " (X=" << targetX << ")" << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
        aKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !dKeyPressed) {
        dKeyPressed = true;
        // Move to right lane
        if (currentLane < 2) {
            currentLane++;
            switch (currentLane) {
                case 0:
                    targetX = LANE_LEFT;
                    break;
                case 1:
                    targetX = LANE_CENTER;
                    break;
                case 2:
                    targetX = LANE_RIGHT;
                    break;
            }
            std::cout << "Switching to lane " << currentLane
                      << " (X=" << targetX << ")" << std::endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
        dKeyPressed = false;
    }

    // Smooth lane switching movement
    if (abs(airplanePos.x - targetX) > 0.1f) {
        airplanePos.x =
            glm::mix(airplanePos.x, targetX, laneChangeSpeed * deltaTime);
        airplaneMoved = true;
    }

    // Optional: Up/Down movement (keep for fine control)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        airplanePos.y += airplaneSpeed * deltaTime;
        airplaneMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        airplanePos.y -= airplaneSpeed * deltaTime;
        airplaneMoved = true;
    }

    // Simple camera following for lane game - camera stays behind plane
    float cameraDistance = 10.0f;  // Distance behind airplane
    float cameraHeight = 3.0f;     // Height above airplane
    float rightOffset = 1.0f;      // Slightly to the right for better lane view

    // Camera follows airplane but stays at fixed relative position
    glm::vec3 idealCameraPos =
        airplanePos +
        glm::vec3(rightOffset - 1.0f, cameraHeight, -cameraDistance);

    // Smooth camera movement
    float followSpeed = 3.0f * deltaTime;
    camera.Position = glm::mix(camera.Position, idealCameraPos, followSpeed);
}

void drawCube(unsigned int VAO, Shader& shader, glm::vec3 position,
              glm::vec3 color) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    shader.setVec3("color", color);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void spawnObstacle() {
    if (obstacles.size() >= maxActiveObstacles) return;

    // Choose a random lane for the obstacle
    int randomLane = rand() % 3;  // 0, 1, or 2
    float obstacleX;

    switch (randomLane) {
        case 0:
            obstacleX = LANE_LEFT;
            break;
        case 1:
            obstacleX = LANE_CENTER;
            break;
        case 2:
            obstacleX = LANE_RIGHT;
            break;
        default:
            obstacleX = LANE_CENTER;
            break;
    }

    // Spawn obstacle ahead of airplane
    glm::vec3 obstaclePos =
        glm::vec3(obstacleX, 0.0f, airplanePos.z + obstacleSpawnDistance);

    obstacles.push_back({obstaclePos, randomLane, true});

    std::cout << "Spawned obstacle in lane " << randomLane
              << " at Z=" << obstaclePos.z << std::endl;
}

void updateObstacles() {
    // Remove obstacles that are far behind the airplane
    for (auto it = obstacles.begin(); it != obstacles.end();) {
        if (it->position.z < airplanePos.z - 10.0f) {
            it = obstacles.erase(it);
        } else {
            ++it;
        }
    }

    // Spawn new obstacles if needed
    if (obstacles.empty() ||
        obstacles.back().position.z <
            airplanePos.z + obstacleSpawnDistance - obstacleSpacing) {
        spawnObstacle();
    }
}

void checkCollisions() {
    for (auto& obstacle : obstacles) {
        if (obstacle.active) {
            // Check distance between airplane and obstacle
            float distance = glm::length(airplanePos - obstacle.position);

            // Collision detection (closer distance since it's lane-based)
            if (distance < 1.5f) {
                std::cout << "COLLISION! Hit obstacle in lane " << obstacle.lane
                          << std::endl;
                std::cout << "Game Over! (In a real game, this would restart "
                             "or show game over screen)"
                          << std::endl;

                // For now, just deactivate the obstacle so game continues
                obstacle.active = false;

                // Optional: Add game over logic here
                // glfwSetWindowShouldClose(window, true);  // Uncomment to quit
                // on collision
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
