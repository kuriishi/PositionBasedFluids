#pragma once

namespace renderer {
    namespace fluid {
        enum DisplayMode {
            FLUID,
            CARTOON,
            FOAM,
            NORMAL,
            THICKNESS,
            DEPTH,
            PARTICLE
        };

        extern DisplayMode displayMode;
        extern bool enableSmoothDepth;
        extern int smoothIteration;
        extern int smoothKernelRadius;
        extern float particleRadiusScaler;
        extern float minimumDensityScaler;
        extern float thicknessScaler;
        extern float fluidColor[3];
        extern bool separateBilateralFilter;
        // cartoon
        extern float brightThreshold;
        extern float brightFactor;
        extern float darkThreshold;
        extern float darkFactor;
        extern float refractThreshold;
        extern float refractMax;
        extern float reflectThreshold;
        extern float reflectMax;
        extern float foamDensityScaler;
        extern int foamErodeKernelRadius;
        extern int foamErodeMinimunNeighborCount;
        extern int edgeSize;
        extern bool enableFixInvalidNormals;
    }
}