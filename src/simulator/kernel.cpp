#include "kernel.hpp"

#include <cmath>

using glm::length;
using glm::dot;
using glm::normalize;

using std::pow;

using common::PI;

namespace simulator::kernel {
    real Poly6(vec3 r, real h) {
        real h2 = h * h;
        real r2 = dot(r, r);
        if (r2 > h2) {
            return 0.0;
        }
        else {
            real fac1 = 315.0 / (64.0 * PI * pow(h, 9));
            real fac2 = pow(h2 - r2, 3);
            return fac1 * fac2;
        }
    }

    real Poly6W(vec3 r, real h) {
        real r_mag = length(r);
        real ratio = r_mag / h;
        real h3 = h * h * h;
        if (ratio <= 0.5) {
            real fac1 = 8.0 / (PI * h3);
            real fac2 = 6.0 * pow(ratio, 3) - 6.0 * pow(ratio, 2) + 1.0;
            return fac1 * fac2;
        }
        else {
            real fac1 = 16.0 / (PI * h3);
            real fac2 = pow(1.0 - ratio, 3);
            return fac1 * fac2;
        }
    }

    real Spiky(vec3 r, real h) {
        real r_mag = length(r);
        if (r_mag > h) {
            return 0.0;
        }
        else {
            real fac1 = 15.0 / (PI * pow(h, 6));
            real fac2 = pow(h - r_mag, 3);
            return fac1 * fac2;
        }
    }

    vec3 SpikyGradient(vec3 r, real h) {
        real r_mag = length(r);
        if (r_mag > h) {
            return vec3(0.0);
        }
        else {
            if (r_mag < 0.00001) {
                return vec3(0.0);
            }
            else {
                real fac1 = -45.0 / (PI * pow(h, 6));
                real fac2 = pow(h - r_mag, 2);
                return normalize(r) * static_cast<float>(fac1 * fac2);
            }
        }
    }

    vec3 SpikyWGradient(vec3 r, real h) {
        real r_mag = length(r);
        real ratio = r_mag / h;
        real h3 = h * h * h;
        if (r_mag > 1.0e-6) {
            if (ratio <= 0.5) {
                real fac1 = 48.0 / (PI * h3);
                real fac2 = 3.0 * pow(ratio, 2) - 2.0 * ratio;
                return static_cast<float>(fac1 * fac2 / h) * normalize(r);
            }
            else {
                real fac1 = 48.0 / (PI * h3);
                real fac2 = pow(1.0 - ratio, 2);
                return static_cast<float>(fac1 * fac2 / -h) * normalize(r);
            }
        }
        else {
            return vec3(0.0);
        }
    }
}
