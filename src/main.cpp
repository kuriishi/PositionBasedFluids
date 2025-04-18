#include "renderer/window.hpp"
#include "renderer/renderer.hpp"
#include "simulator/simulator.hpp"
#include "common/performance_log.hpp"
#include "gui/gui.hpp"

#include <iostream>
#include <iomanip>

int main() {
    renderer::windowInit();
    renderer::renderInit();
    simulator::simulateInit();
    common::performanceLogInit();
    gui::guiInit();

    while(!glfwWindowShouldClose(renderer::window)) {
        if (common::resetSimulation) {
            common::resetSimulation = false;
            simulator::simulateTerminate();
            simulator::simulateInit();
        }

        {
        common::queryTime(0);
        }

        if (common::enableSimulation) {
            simulator::simulate();
        }

        {
        common::queryTime(common::SIMULATE_TIME_QUERY_COUNT);
        }

        glfwPollEvents();
        renderer::computeDeltaTime();
        renderer::processInput(renderer::window);
        if (common::enableCameraMovement) {
            renderer::enableMouseMovement();
            glfwSetInputMode(renderer::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            renderer::processCameraInput(renderer::window);
        }
        else {
            renderer::disableMouseMovement();
            glfwSetInputMode(renderer::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        {
        common::queryTime(common::SIMULATE_TIME_QUERY_COUNT + 1);
        }

        renderer::render();
        gui::guiRender(); 
        glfwSwapBuffers(renderer::window);

        {
        common::queryTime(common::TIME_QUERY_COUNT - 1);
        }


        common::updateFrameCount();
    }

    
    gui::guiTerminate();
    common::performanceLogTerminate();
    simulator::simulateTerminate();
    renderer::renderTerminate();
    renderer::windowTerminate();

    return 0;
}
