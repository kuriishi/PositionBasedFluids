#pragma once

#define ENABLE_PRINT_RENDER_PERFORMANCE
#define ENABLE_PRINT_SIMULATE_PERFORMANCE

#include <string>
#include <iostream>

namespace common {
    const unsigned int FRAME_COUNT_PER_LOG = 32;
    const double MILLISECONDS_SCALER = 1e-6;

    const unsigned int SIMULATE_TIME_QUERY_COUNT = 8;
    const unsigned int RENDER_TIME_QUERY_COUNT = 9;
    const unsigned int TIME_QUERY_COUNT = RENDER_TIME_QUERY_COUNT + SIMULATE_TIME_QUERY_COUNT + 1;

    const std::string PERFORMANCE_LOG_FILE_NAME = "performance_log.txt";

    extern double fps;
    extern double totalTime;
    extern double renderTime;
    extern double simulateTime;

    extern double renderTimeSlice[RENDER_TIME_QUERY_COUNT];
    extern double renderTimePercentage[RENDER_TIME_QUERY_COUNT];

    extern double simulateTimeSlice[SIMULATE_TIME_QUERY_COUNT];
    extern double simulateTimePercentage[SIMULATE_TIME_QUERY_COUNT];

    int performanceLogInit();
    int performanceLogTerminate();
    void queryTime(unsigned int index);
    void calculateTime();
    void outputRenderPerformance(std::ostream& os);
    void outputSimulatePerformance(std::ostream& os);
    void outputPerformance(std::ostream& os);
    void printPerformanceToConsole();
    void printPerformanceToFile();

    unsigned int getFrameCount();
    void updateFrameCount();

    std::string getSupplementarySymbol(char symbol);
}
