#include "window.hpp"

#include <glm/glm.hpp>

#include <iostream>

#include "camera.hpp"

using glm::vec3;

using std::cout;
using std::endl;

namespace renderer {
    GLFWwindow* window;

    // camera
    Camera camera(vec3(0.0f, 0.75f, 2.5f));
    real lastX = SCR_WIDTH / 2.0;
    real lastY = SCR_HEIGHT / 2.0;
    bool firstMouse = true;
    real lastFrame = 0.0;
    real deltaTime = 0.0;

    int windowInit() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Position Based Fluid", NULL, NULL);
        if (window == NULL)
        {
            cout << "Failed to create GLFW window" << endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            cout << "Failed to initialize GLAD" << endl;
            return -1;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug_callback, nullptr);

        return 0;
    }

    int windowTerminate() {
        glfwTerminate();

        return 0;
    }

    real computeDeltaTime() {
        real currentFrame = static_cast<real>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        return deltaTime;
    }

    void processInput(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, static_cast<float>(deltaTime));
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, static_cast<float>(deltaTime));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, static_cast<float>(deltaTime));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, static_cast<float>(deltaTime));
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, static_cast<float>(deltaTime));
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, static_cast<float>(deltaTime));
    }

    bool mouseMovement = true;
    void enableMouseMovement() {
        mouseMovement = true;
    }
    void disableMouseMovement() {
        mouseMovement = false;
    }
    void mouse_callback(GLFWwindow* window, real xpos, real ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        real xoffset = xpos - lastX;
        real yoffset = lastY - ypos;
        
        lastX = xpos;
        lastY = ypos;

        if (mouseMovement) {
            camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
        }
    }

    void scroll_callback(GLFWwindow* window, real xoffset, real yoffset) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }

    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        cout << "[GL Debug] " << message << endl;
    }
}
