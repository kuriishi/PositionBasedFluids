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

    int renderInitParticle();
    int renderParticle();
    int renderTerminateParticle();

    int renderInitSkybox();
    int renderSkybox();
    int renderTerminateSkybox();

    unsigned int loadTexture(const char* path);
    unsigned int loadCubemap(vector<string> faces);
}
