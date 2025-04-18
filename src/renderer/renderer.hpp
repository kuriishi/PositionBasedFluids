#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "../common/common.hpp"

namespace renderer {
    enum DisplayMode {
        FLUID,
        NORMAL,
        THICKNESS,
        DEPTH,
        PARTICLE
    };
    // gui parameters
    extern DisplayMode displayMode;
    extern bool enableSmoothDepth;
    extern int smoothIteration;
    extern float particleRadiusScaler;
    extern float minimumDensityScaler;
    extern float thicknessScaler;
    extern float fluidColor[3];

    // renderer
    int renderInit();
    int render();
    int renderTerminate();

    // Fluid
    int renderInitFluid();
    int clearFlag();
    int renderFluidDepthTexture();
    int renderFluidThicknessTexture();
    int smoothFluidDepthTexture();
    int computeFluidNormalTexture();
    int renderFluidPrepare();

    int renderFluid();
    int renderFluidDepth();
    int renderFluidThickness();
    int renderFluidNormal();
    int renderParticle();

    int renderTerminateFluid();

    // Background
    int renderInitSkybox();
    int renderSkybox();
    int renderTerminateSkybox();

    int renderInitFloor();
    int renderFloor();
    int renderTerminateFloor();

    int renderInitBackground();
    int renderBackground();
    int drawBackground();
    int renderTerminateBackground();

    // utils
    int copyParticleAttribute();

    unsigned int loadTexture(const char* path);
    unsigned int loadCubemap(std::vector<std::string> faces);
}
