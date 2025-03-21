#include "renderer/window.hpp"
#include "renderer/renderer.hpp"
#include "simulator/simulator.hpp"

using renderer::renderInit;
using renderer::render;
using renderer::renderTerminate;
using renderer::window;

using simulator::simulateInit;
using simulator::simulate;
using simulator::simulateTerminate;

#include <iostream>

int main() {
    simulateInit();
    renderInit();

    while (!glfwWindowShouldClose(window)) {
        render();
        simulate();
    }

    simulateTerminate();
    renderTerminate();

    return 0;
}
