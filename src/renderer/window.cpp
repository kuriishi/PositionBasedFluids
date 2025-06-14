#include "window.hpp"

#include <glm/glm.hpp>

#include <iostream>

#include "camera.hpp"

#include "../simulator/simulator.hpp"

namespace renderer {
    namespace window {
        GLFWwindow* window;

        // camera
        Camera camera(glm::vec3(0.0f, 1.5f, -2.5f), glm::vec3(0.0f, 0.0f, 1.0f));
        common::real lastX = SCR_WIDTH / 1.5;
        common::real lastY = SCR_HEIGHT / 1.5;
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
            // sync
            glfwSwapInterval(1);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
            glfwSetCursorPosCallback(window, mouse_callback);
            // glfwSetScrollCallback(window, scroll_callback);

            disableMouseMovement();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                return -1;
            }

            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(debug_callback, nullptr);
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
            static bool pressedP = false;
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                if (!pressedP) {
                    pressedP = true;
                    common::hideGUI = !common::hideGUI;
                }
            }
            else {
                pressedP = false;
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

            // camera mode
            static bool pressedC = false;
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                if (!pressedC) {
                    pressedC = true;
                    common::cameraMode = !common::cameraMode;
                }   
            }
            else {
                pressedC = false;
            }

            // exit
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }

            // manipulate velocity
            static bool pressedJ = false;
            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
                if (!pressedJ) {
                    pressedJ = true;
                    simulator::uDown = 1;
                }
            }
            else {
                pressedJ = false;
                simulator::uDown = 0;
            }

            static bool pressedK = false;
            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
                if (!pressedK) {
                    pressedK = true;
                    simulator::uUp = 1;
                }
            }
            else {
                pressedK = false;
                simulator::uUp = 0;
            }

            static bool pressedL = false;
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
                if (!pressedL) {
                    pressedL = true;
                    simulator::uRight = 1;
                }
            }
            else {
                pressedL = false;
                simulator::uRight = 0;
            }

            static bool pressedH = false;
            if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
                if (!pressedH) {
                    pressedH = true;
                    simulator::uLeft = 1;
                }
            }
            else {
                pressedH = false;
                simulator::uLeft = 0;
            }

            static bool pressedI = false;
            if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
                if (!pressedI) {
                    pressedI = true;
                    simulator::uFront = 1;
                }
            }
            else {
                pressedI = false;
                simulator::uFront = 0;
            }

            static bool pressedO = false;
            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
                if (!pressedO) {
                    pressedO = true;
                    simulator::uBack = 1;
                }
            }
            else {
                pressedO = false;
                simulator::uBack = 0;
            }
        }

        void processCameraInput(GLFWwindow *window) {
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

        // void scroll_callback(GLFWwindow* window, common::real xoffset, common::real yoffset) {
        //     camera.ProcessMouseScroll(static_cast<float>(yoffset));
        // }

        void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }

        void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
            std::cerr << "[GL Debug] " << message << std::endl;
        }
    }
}
