#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "../common/common.hpp"

using glm::vec3;
using glm::ivec3;

using std::vector;

using common::real;
using common::PI;

namespace simulator {
    extern vector<vec3> particlePositions;

    // simulation parameters
    const unsigned int PARTICLE_COUNT_PER_EDGE_XZ = 16;
    const unsigned int PARTICLE_COUNT_PER_EDGE_Y = 16;
    const unsigned int PARTICLE_COUNT = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_Y;
    const real PARTICLE_RADIUS = 0.01;
    const real HORIZON_MAX_COORDINATE = (PARTICLE_COUNT_PER_EDGE_XZ + 5) * PARTICLE_RADIUS * 3.0;
    const unsigned int ITERATION = 5;
    const real DELTA_TIME = 0.0025;
    const real DELTA_TIME_REVERSE = 1.0 / DELTA_TIME;

    const real KERNEL_RADIUS = PARTICLE_RADIUS * 4.0;
    const real REST_DENSITY = 1000.0;
    const real REST_DENSITY_REVERSE = 1.0 / REST_DENSITY;
    const real RELAXATION_PARAMETER = 1.0e-6;

    const vec3 GRAVITY = vec3(0.0, -9.8, 0.0);
    const real RESTITUTION = 1.0;
    const real FRICTION = 1.0;
    // const real MASS = 1.0;
    // const real MASS = 4.0 / 3.0 * PI * PARTICLE_RADIUS * PARTICLE_RADIUS * PARTICLE_RADIUS * REST_DENSITY;
    const real MASS = 6.4 * PARTICLE_RADIUS * PARTICLE_RADIUS * PARTICLE_RADIUS * REST_DENSITY;
    const real MASS_REVERSE = 1.0 / MASS;

    const real VISCOSITY_PARAMETER = 0.008;
    const real VORTICITY_PARAMETER = 0.00000015;

    const unsigned int DEBUG_INDEX = PARTICLE_COUNT / 2;

    int simulateInit();
    int simulate();
    int simulateTerminate();

    int computeNeighbors();
    real computeLambda(const unsigned int index);
    vec3 computeDeltaPosition(const unsigned int index);

    int boundaryCollisionHandling(const unsigned int index);
    int applyVorticityConfinement();
    int applyViscosity();

    ivec3 getCubeIndex(const vec3& position);
    real computeConstraint(const unsigned int index);
    real computeConstraintGradSquareSum(const unsigned int index);
    int computeDensity();
    int computeCurl();
}
