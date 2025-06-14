#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define eGPU

namespace common {
    using real = double;
    inline constexpr real PI = 3.14159265358979323846;
    const unsigned int INVOCATION_PER_WORKGROUP = 256;
    inline constexpr real EPSILON = 1e-4;

    // utils
    extern bool enableSimulation;
    extern bool resetSimulation;
    extern bool hideGUI;
    extern bool cameraMode;

}
