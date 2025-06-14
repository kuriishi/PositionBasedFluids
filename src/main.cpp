#include "renderer/window.hpp"
#include "renderer/renderer.hpp"
#include "simulator/simulator.hpp"
#include "common/performance_log.hpp"
#include "gui/gui.hpp"

#include <iostream>
#include <iomanip>

int main() {
    renderer::Renderer renderer;
    simulator::simulateInit();
    common::performanceLogInit();
    gui::guiInit();

    while(!glfwWindowShouldClose(renderer::window::window)) {
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
        renderer::window::computeDeltaTime();
        renderer::window::processInput(renderer::window::window);
        if (common::cameraMode) {
            renderer::window::enableMouseMovement();
            glfwSetInputMode(renderer::window::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            renderer::window::processCameraInput(renderer::window::window);
        }
        else {
            renderer::window::disableMouseMovement();
            glfwSetInputMode(renderer::window::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        {
        common::queryTime(common::SIMULATE_TIME_QUERY_COUNT + 1);
        }

        renderer.render();
        gui::guiRender(); 
        glfwSwapBuffers(renderer::window::window);

        {
        common::queryTime(common::TIME_QUERY_COUNT - 1);
        }


        common::updateFrameCount();
    }

    
    gui::guiTerminate();
    common::performanceLogTerminate();
    simulator::simulateTerminate();
    renderer::window::windowTerminate();

    return 0;
}
