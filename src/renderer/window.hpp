#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../common/common.hpp"
#include "camera.hpp"

namespace renderer {
    namespace window {
        extern GLFWwindow* window;
        extern common::real deltaTime;
        extern Camera camera;

        const unsigned int SCR_WIDTH = 1920;
        const unsigned int SCR_HEIGHT = 1080;

        int windowInit();
        int windowTerminate();

        common::real computeDeltaTime();

        void processInput(GLFWwindow *window);
        void processCameraInput(GLFWwindow *window);
        void scroll_callback(GLFWwindow* window, common::real xoffset, common::real yoffset);
        void mouse_callback(GLFWwindow* window, common::real xpos, common::real ypos);
        void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

        void enableMouseMovement();
        void disableMouseMovement();
    }
}
