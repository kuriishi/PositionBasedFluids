#pragma once

#include <glm/glm.hpp>

#include "../common/common.hpp"

using common::real;
using glm::vec3;

namespace simulator::kernel {
    real Poly6(vec3 r, real h);
    real Poly6W(vec3 r, real h);
    real Spiky(vec3 r, real h);
    vec3 SpikyGradient(vec3 r, real h);
    vec3 SpikyWGradient(vec3 r, real h);
}
