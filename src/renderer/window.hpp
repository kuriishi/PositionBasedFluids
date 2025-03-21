#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../common/common.hpp"
#include "camera.hpp"

using common::real;

namespace renderer {
    extern GLFWwindow* window;
    extern real deltaTime;
    extern Camera camera;
    const unsigned int SCR_WIDTH = 1920;
    const unsigned int SCR_HEIGHT = 1080;

    int windowInit();
    int windowTerminate();

    real computeDeltaTime();

    void processInput(GLFWwindow *window);
    void scroll_callback(GLFWwindow* window, real xoffset, real yoffset);
    void mouse_callback(GLFWwindow* window, real xpos, real ypos);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
}
