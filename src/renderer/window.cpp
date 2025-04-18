#include "window.hpp"

#include <glm/glm.hpp>

#include <iostream>

#include "camera.hpp"

namespace renderer {
    GLFWwindow* window;

    // camera
    Camera camera(glm::vec3(0.0f, 1.5f, 2.5f), glm::vec3(0.0f, 0.0f, -1.0f));
    common::real lastX = SCR_WIDTH / 2.0;
    common::real lastY = SCR_HEIGHT / 2.0;
    bool firstMouse = true;
    common::real lastFrame = 0.0;
    common::real deltaTime = 0.0;

    int windowInit() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Position Based Fluid", NULL, NULL);
        if (window == NULL)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        disableMouseMovement();
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        // glEnable(GL_DEBUG_OUTPUT);
        // glDebugMessageCallback(debug_callback, nullptr);
        glDisable(GL_DEBUG_OUTPUT);

        glFinish();

        return 0;
    }

    int windowTerminate() {
        glfwTerminate();

        glFinish();

        return 0;
    }

    common::real computeDeltaTime() {
        common::real currentFrame = static_cast<common::real>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        return deltaTime;
    }

    void processInput(GLFWwindow *window) {
        // hide GUI
        static bool pressedH = false;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!pressedH) {
                pressedH = true;
                common::hideGUI = !common::hideGUI;
            }
        }
        else {
            pressedH = false;
        }

        // reset simulation
        static bool pressedR = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!pressedR) {
                pressedR = true;
                common::resetSimulation = true;
            }
        }
        else {
            pressedR = false;
        }

        // stop simulation
        static bool pressedQ = false;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            if (!pressedQ) {
                pressedQ = true;
                common::enableSimulation = !common::enableSimulation;
            }
        }
        else {
            pressedQ = false;
        }
    }

    void processCameraInput(GLFWwindow *window) {
        // escape
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            common::enableCameraMovement = false;
        }

        // move
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
    void mouse_callback(GLFWwindow* window, common::real xpos, common::real ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        common::real xoffset = xpos - lastX;
        common::real yoffset = lastY - ypos;
        
        lastX = xpos;
        lastY = ypos;

        if (mouseMovement) {
            camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
        }
    }

    void scroll_callback(GLFWwindow* window, common::real xoffset, common::real yoffset) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }

    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        std::cerr << "[GL Debug] " << message << std::endl;
    }
}
