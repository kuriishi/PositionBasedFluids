#pragma once

namespace common {
    using real = double;
    inline constexpr real PI = 3.14159265358979323846;
    const unsigned int INVOCATION_PER_WORKGROUP = 256;

    // utils
    extern bool enableCameraMovement;
    extern bool enableSimulation;
    extern bool resetSimulation;
    extern bool hideGUI;

}
