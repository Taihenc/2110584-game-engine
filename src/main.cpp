#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <learnopengl/animator.h>
#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/shader_m.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 characterPosition = glm::vec3(0.0f, -0.6f, 0.0f);

// Key mapping for actions - change these to remap controls easily
// By default: Idle = 1, Walk = W, Run = R, Jump = Space
const int KEY_ACTION_IDLE = GLFW_KEY_1;
const int KEY_ACTION_WALK = GLFW_KEY_W;
const int KEY_ACTION_RUN = GLFW_KEY_R;
const int KEY_ACTION_JUMP = GLFW_KEY_SPACE;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading
    // model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("anim_model.vs", "anim_model.fs");

    // load models
    // -----------
    Model ourModel(FileSystem::getPath("resources/objects/maria/Idle.dae"));
    Animation idleAnimation(
        FileSystem::getPath("resources/objects/maria/Idle.dae"), &ourModel);
    Animation walkAnimation(
        FileSystem::getPath("resources/objects/maria/Walking.dae"), &ourModel);
    Animation runAnimation(
        FileSystem::getPath("resources/objects/maria/Fast Run.dae"), &ourModel);
    // Animation
    // stepAnimation(FileSystem::getPath("resources/objects/wiz/step-mixamo/Standing
    // Dodge Backward.dae"), &ourModel);
    Animation jumpAnimation(
        FileSystem::getPath("resources/objects/maria/Jump.dae"), &ourModel);
    Animator animator(&idleAnimation);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        // direct number key shortcuts (primary mappings)
        if (glfwGetKey(window, KEY_ACTION_IDLE) == GLFW_PRESS)
            animator.PlayAnimation(&idleAnimation);
        if (glfwGetKey(window, KEY_ACTION_WALK) == GLFW_PRESS)
            animator.PlayAnimation(&walkAnimation);
        if (glfwGetKey(window, KEY_ACTION_RUN) == GLFW_PRESS)
            animator.PlayAnimation(&runAnimation);
        if (glfwGetKey(window, KEY_ACTION_JUMP) == GLFW_PRESS)
            animator.PlayAnimation(&jumpAnimation);

        // simple single-animation controls (the provided Animator has a minimal
        // API)
        // alternative keys (kept for convenience)
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            animator.PlayAnimation(&walkAnimation);  // E => walk (alt)
        if (glfwGetKey(window, KEY_ACTION_JUMP) == GLFW_PRESS)
            animator.PlayAnimation(
                &jumpAnimation);  // Space => jump (same as KEY_ACTION_JUMP)
        if (glfwGetKey(window, KEY_ACTION_RUN) == GLFW_PRESS)
            animator.PlayAnimation(
                &runAnimation);  // R => run (same as KEY_ACTION_RUN)

        animator.UpdateAnimation(deltaTime);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // Debug: print active key mapping once
        static bool printedMappings = false;
        if (!printedMappings) {
            std::cout << "Key mappings: Idle=" << KEY_ACTION_IDLE
                      << " Walk=" << KEY_ACTION_WALK
                      << " Run=" << KEY_ACTION_RUN
                      << " Jump=" << KEY_ACTION_JUMP << std::endl;
            printedMappings = true;
        }

        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                              transforms[i]);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(
            model, characterPosition);  // translate it down so it's at the
                                        // center of the scene
        model = glm::scale(
            model,
            glm::vec3(
                .75f, .75f,
                .75f));  // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset =
        lastY - ypos;  // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
