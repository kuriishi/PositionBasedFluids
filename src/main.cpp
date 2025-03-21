#include "renderer/window.hpp"
#include "renderer/renderer.hpp"
#include "simulator/simulator.hpp"

#include <iostream>
#include <iomanip>

using renderer::renderInit;
using renderer::render;
using renderer::renderTerminate;
using renderer::window;
using renderer::windowInit;
using renderer::windowTerminate;
using renderer::disableMouseMovement;
using simulator::simulateInit;
using simulator::simulate;
using simulator::simulateTerminate;

using std::cin;
using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::flush;

// #define ENABLE_PRESS_TO_START
#define ENABLE_PRINT_PERFORMANCE_LOG

#ifdef ENABLE_PRINT_PERFORMANCE_LOG
GLuint64 frameStartTime, renderEndTime, simulateEndTime;
GLuint timeQueryID[3];
const unsigned int FRAME_COUNT_PER_LOG = 32;
unsigned int frameCount = 0;
void printPerformance(double renderTime, double simulateTime);
#endif

int main() {
    windowInit();
    renderInit();
    simulateInit();

    #ifdef ENABLE_PRESS_TO_START
    disableMouseMovement();
    cout << "Press ENTER to start simulation" << endl;
    cin.get();
    #endif

    while (!glfwWindowShouldClose(window)) {
        #ifdef ENABLE_PRINT_PERFORMANCE_LOG
        glGenQueries(3, timeQueryID);
        glQueryCounter(timeQueryID[0], GL_TIMESTAMP);
        #endif

        render();

        #ifdef ENABLE_PRINT_PERFORMANCE_LOG
        glQueryCounter(timeQueryID[1], GL_TIMESTAMP);
        #endif

        simulate();

        #ifdef ENABLE_PRINT_PERFORMANCE_LOG
        glQueryCounter(timeQueryID[2], GL_TIMESTAMP);
        #endif

        #ifdef ENABLE_PRINT_PERFORMANCE_LOG
        frameCount++;
        if (frameCount % FRAME_COUNT_PER_LOG == 0) {
            glGetQueryObjectui64v(timeQueryID[0], GL_QUERY_RESULT, &frameStartTime);
            glGetQueryObjectui64v(timeQueryID[1], GL_QUERY_RESULT, &renderEndTime);
            glGetQueryObjectui64v(timeQueryID[2], GL_QUERY_RESULT, &simulateEndTime);

            // convert to milliseconds
            double renderTime = static_cast<double>(renderEndTime - frameStartTime) / 1e6;
            double simulateTime = static_cast<double>(simulateEndTime - renderEndTime) / 1e6;
            printPerformance(renderTime, simulateTime);
        }
        #endif
    }

    simulateTerminate();
    renderTerminate();
    windowTerminate();

    return 0;
}

#ifdef ENABLE_PRINT_PERFORMANCE_LOG
void printPerformance(double renderTime, double simulateTime) {
    double totalTime = renderTime + simulateTime;
    double fps = 1000.0 / totalTime;
    double renderPercent = renderTime / totalTime * 100.0;
    double simulatePercent = simulateTime / totalTime * 100.0;

    cout << fixed << setprecision(2);
    cout << "==========Performance(" << frameCount << "th frame)===========\n"
         << "Frame: \t\t" << totalTime << " ms \t(" << fps << " FPS)\n"
         << "Render: \t" << renderTime << " ms \t(" << renderPercent << " %)\n"
         << "Simulate: \t" << simulateTime << " ms \t(" << simulatePercent << " %)\n"
         << "==========================================";
    unsigned int temp = frameCount;
    while (temp > 0) {
        cout << "=";
        temp /= 10;
    }
    cout << "\n" << flush;
}
#endif
