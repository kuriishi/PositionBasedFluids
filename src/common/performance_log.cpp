#include "performance_log.hpp"

#include <glad/glad.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <ctime>

#include "common.hpp"

namespace common {
    unsigned int frameCount = 0;
    GLuint timeQueryID[TIME_QUERY_COUNT];
    double timeMarker[TIME_QUERY_COUNT];

    double totalTime;
    double fps;

    double renderTime;
    double renderTimeSlice[RENDER_TIME_QUERY_COUNT];
    double renderTimePercentage[RENDER_TIME_QUERY_COUNT];

    double simulateTime;
    double simulateTimeSlice[SIMULATE_TIME_QUERY_COUNT];
    double simulateTimePercentage[SIMULATE_TIME_QUERY_COUNT];

    int performanceLogInit() {
        glGenQueries(TIME_QUERY_COUNT, timeQueryID);

        std::ofstream clearFile(PERFORMANCE_LOG_FILE_NAME, std::ios::trunc);
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        struct tm buf;
        #ifdef _WIN32
            localtime_s(&buf, &in_time_t);
        #else
            localtime_r(&in_time_t, &buf);
        #endif
        clearFile << "Performance Log\n" << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << "\n\n";
        clearFile.close();

        glFinish();

        return 0;
    }

    int performanceLogTerminate() {
        glDeleteQueries(TIME_QUERY_COUNT, timeQueryID);

        glFinish();

        return 0;
    }

    void queryTime(unsigned int index) {
        glQueryCounter(timeQueryID[index], GL_TIMESTAMP);
    }

    void calculateTime() {
        for (unsigned int i = 0; i < TIME_QUERY_COUNT; i++) {
            GLuint64 time;
            glGetQueryObjectui64v(timeQueryID[i], GL_QUERY_RESULT, &time);
            timeMarker[i] = static_cast<double>(time) * MILLISECONDS_SCALER;
        }

        if (enableSimulation) {
            totalTime = timeMarker[TIME_QUERY_COUNT - 1] - timeMarker[0];
        }
        else {
            totalTime = timeMarker[TIME_QUERY_COUNT - 1] - timeMarker[SIMULATE_TIME_QUERY_COUNT];
        }
        fps = 1000.0 / totalTime;

        // simulate time
        if (enableSimulation) {
            simulateTime = timeMarker[SIMULATE_TIME_QUERY_COUNT] - timeMarker[0];
            for (unsigned int i = 0; i < SIMULATE_TIME_QUERY_COUNT; i++) {
                simulateTimeSlice[i] = timeMarker[i + 1] - timeMarker[i];
            }
            for (unsigned int i = 0; i < SIMULATE_TIME_QUERY_COUNT; i++) {
                simulateTimePercentage[i] = simulateTimeSlice[i] / simulateTime * 100;
            }
        }
        else {
            simulateTime = 0.0;
            for (unsigned int i = 0; i < SIMULATE_TIME_QUERY_COUNT; i++) {
                simulateTimeSlice[i] = 0.0;
            }
            for (unsigned int i = 0; i < SIMULATE_TIME_QUERY_COUNT; i++) {
                simulateTimePercentage[i] = 0.0;
            }
        }

        // render time
        renderTime = timeMarker[TIME_QUERY_COUNT - 1] - timeMarker[SIMULATE_TIME_QUERY_COUNT];
        for (unsigned int i = 0; i < RENDER_TIME_QUERY_COUNT; i++) {
            renderTimeSlice[i] = timeMarker[i + 1 + SIMULATE_TIME_QUERY_COUNT] - timeMarker[i + SIMULATE_TIME_QUERY_COUNT];
        }
        for (unsigned int i = 0; i < RENDER_TIME_QUERY_COUNT; i++) {
            renderTimePercentage[i] = renderTimeSlice[i] / renderTime * 100;
        }
    }

    void outputRenderPerformance(std::ostream& os) {
        os << std::fixed << std::setprecision(2);
        os << "--------------------Render Performance------------------" << getSupplementarySymbol('-') << "\n"
             << "Render: \t\t\t\t\t\t" << std::setw(5) << renderTime << " ms \n"
             << "Process Input: \t\t\t\t\t" << std::setw(5) << renderTimeSlice[0] << " ms \t( " << std::setw(5) << renderTimePercentage[0] << " %)\n"
             << "Background: \t\t\t\t\t" << std::setw(5) << renderTimeSlice[1] << " ms \t( " << std::setw(5) << renderTimePercentage[1] << " %)\n"
             << "Copy Particle Attribute: \t\t" << std::setw(5) << renderTimeSlice[2] << " ms \t( " << std::setw(5) << renderTimePercentage[2] << " %)\n"
             << "Fluid Depth: \t\t\t\t\t" << std::setw(5) << renderTimeSlice[3] << " ms \t( " << std::setw(5) << renderTimePercentage[3] << " %)\n"
             << "Smooth Fluid Depth: \t\t\t" << std::setw(5) << renderTimeSlice[4] << " ms \t( " << std::setw(5) << renderTimePercentage[4] << " %)\n"
             << "Compute Fluid Normal: \t\t\t" << std::setw(5) << renderTimeSlice[5] << " ms \t( " << std::setw(5) << renderTimePercentage[5] << " %)\n"
             << "Fluid Thickness: \t\t\t\t" << std::setw(5) << renderTimeSlice[6] << " ms \t( " << std::setw(5) << renderTimePercentage[6] << " %)\n"
             << "Render Final Scene: \t\t\t" << std::setw(5) << renderTimeSlice[7] << " ms \t( " << std::setw(5) << renderTimePercentage[7] << " %)\n"
             << "Swap Buffers: \t\t\t\t\t" << std::setw(5) << renderTimeSlice[8] << " ms \t( " << std::setw(5) << renderTimePercentage[8] << " %)\n"
             << "--------------------------------------------------------" << getSupplementarySymbol('-') << "\n"
             << std::flush;
    }

    void outputSimulatePerformance(std::ostream& os) {
        os << std::fixed << std::setprecision(2);
        os << "---------------------Simulate Performance---------------" << getSupplementarySymbol('-') << "\n"
             << "Simulate: \t\t\t\t\t\t" << std::setw(5) << simulateTime << " ms \n"
             << "Apply External Force: \t\t\t" << std::setw(5) << simulateTimeSlice[0] << " ms \t( " << std::setw(5) << simulateTimePercentage[0] << " %)\n"
             << "Search Neighbor: \t\t\t\t" << std::setw(5) << simulateTimeSlice[1] << " ms \t( " << std::setw(5) << simulateTimePercentage[1] << " %)\n"
             << "Constraint Projection: \t\t\t" << std::setw(5) << simulateTimeSlice[2] << " ms \t( " << std::setw(5) << simulateTimePercentage[2] << " %)\n"
             << "Update Velocity by Position: \t" << std::setw(5) << simulateTimeSlice[3] << " ms \t( " << std::setw(5) << simulateTimePercentage[3] << " %)\n"
             << "Vorticity Confinement: \t\t\t" << std::setw(5) << simulateTimeSlice[4] << " ms \t( " << std::setw(5) << simulateTimePercentage[4] << " %)\n"
             << "Viscosity: \t\t\t\t\t\t" << std::setw(5) << simulateTimeSlice[5] << " ms \t( " << std::setw(5) << simulateTimePercentage[5] << " %)\n"
             << "Handle Boundary Collision: \t\t" << std::setw(5) << simulateTimeSlice[6] << " ms \t( " << std::setw(5) << simulateTimePercentage[6] << " %)\n"
             << "Update Particle Position: \t\t" << std::setw(5) << simulateTimeSlice[7] << " ms \t( " << std::setw(5) << simulateTimePercentage[7] << " %)\n"
             << "--------------------------------------------------------" << getSupplementarySymbol('-') << "\n"
             << std::flush;
    }

    void outputPerformance(std::ostream& os) {
        os << std::fixed << std::setprecision(2);
        os << "========================================================" << getSupplementarySymbol('=') << "\n";
        os << "-------------------Performance(" << frameCount << "th frame)----------------\n"
             << "Total: \t\t\t\t\t\t\t" << std::setw(5) << totalTime << " ms \t" << std::setw(6) << fps << " fps\n"
             << "Render: \t\t\t\t\t\t" << std::setw(5) << renderTime << " ms \t( " << std::setw(5) << renderTime / totalTime * 100 << " %)\n"
             << "Simulate: \t\t\t\t\t\t" << std::setw(5) << simulateTime << " ms \t( " << std::setw(5) << simulateTime / totalTime * 100 << " %)\n"
             << "--------------------------------------------------------" << getSupplementarySymbol('-') << "\n"
             << std::flush;

        {
        #ifdef ENABLE_PRINT_RENDER_PERFORMANCE
        outputRenderPerformance(os);
        #endif
        }

        {
        #ifdef ENABLE_PRINT_SIMULATE_PERFORMANCE
        outputSimulatePerformance(os);
        #endif
        }

        os << "========================================================" << getSupplementarySymbol('=') << "\n\n"
             << std::flush;
    }

    void printPerformanceToConsole() {
        outputPerformance(std::cout);
    }

    void printPerformanceToFile() {
        std::ofstream file(PERFORMANCE_LOG_FILE_NAME, std::ios::app);
        outputPerformance(file);
    }

    unsigned int getFrameCount() {
        return frameCount;
    }

    void updateFrameCount() {
        frameCount++;
        calculateTime();
    }

    std::string getSupplementarySymbol(char symbol) {
        unsigned int symbolCount = 0;
        unsigned int tmp = frameCount;
        while (tmp > 0) {
            tmp /= 10;
            symbolCount++;
        }
        std::string result(symbolCount, symbol);
        return result;
    }
}
