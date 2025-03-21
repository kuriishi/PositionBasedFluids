#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "../common/common.hpp"

using glm::vec3;

using std::vector;
using std::string;

namespace renderer {
    int renderInit();
    int render();
    int renderTerminate();

    int generateSphere(const unsigned int SEGMENTS);
    int renderInitParticles();
    int renderParticles();
    int renderTerminateParticles();

    int renderInitSkybox();
    int renderSkybox();
    int renderTerminateSkybox();

    unsigned int loadTexture(const char* path);
    unsigned int loadCubemap(vector<string> faces);
}
