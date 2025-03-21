#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "../common/common.hpp"

using glm::vec3;

using common::real;

namespace simulator {
    extern GLuint particlePositionSSBO;

    const unsigned int PARTICLE_COUNT_PER_EDGE_XZ = 16;
    const unsigned int PARTICLE_COUNT_PER_EDGE_Y = 128;
    const unsigned int PARTICLE_COUNT = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_Y;
    const real PARTICLE_RADIUS = 0.01;
    const real HORIZON_MAX_COORDINATE = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_RADIUS * 2.0 * 5.0;
    const real MAX_HEIGHT = (PARTICLE_COUNT_PER_EDGE_Y + 100) * PARTICLE_RADIUS * 2.0;
    const unsigned int ITERATION = 4;
    const real DELTA_TIME = 0.0016;
    // const real DELTA_TIME = 0.0166666667;
    const real DELTA_TIME_REVERSE = 1.0 / DELTA_TIME;

    const real KERNEL_RADIUS = PARTICLE_RADIUS * 4.0;
    const real REST_DENSITY = 1000.0;
    const real REST_DENSITY_REVERSE = 1.0 / REST_DENSITY;
    const real RELAXATION_PARAMETER = 1.0e-6;

    const vec3 GRAVITY = vec3(0.0, -9.8, 0.0);
    // const real RESTITUTION = 0.8;
    // const real FRICTION = 0.7;
    const real RESTITUTION = 1.0;
    const real FRICTION = 1.0;
    // const real MASS = 1.0;
    // const real MASS = 4.0 / 3.0 * PI * PARTICLE_RADIUS * PARTICLE_RADIUS * PARTICLE_RADIUS * REST_DENSITY;
    const real MASS = 6.4 * PARTICLE_RADIUS * PARTICLE_RADIUS * PARTICLE_RADIUS * REST_DENSITY;
    const real MASS_REVERSE = 1.0 / MASS;

    const real VISCOSITY_PARAMETER = 0.01;
    const real VORTICITY_PARAMETER = 0.0000006;
    // const real VORTICITY_PARAMETER = 0.000001;
    // const real VORTICITY_PARAMETER = 0.0;


    const unsigned int DEBUG_INDEX = PARTICLE_COUNT / 2;

    int simulateInit();
    int simulate();
    int simulateTerminate();

    int applyExternalForce();

    int searchNeighbor();
    int computeLambda();
    int computeDeltaPosition();
    int handleBoundaryCollision();
    int adjustPositionPredict();
    int updateVelocityByPosition();

    int applyVorticityConfinement();
    int applyViscosity();

    int updateParticlePosition();

    int computeDensity();
    int computeConstraint();
    int computeConstraintGradSquareSum();

    int divideCube();
    int clearParticleCountPerCube();
    int computeParticleCountPerCube();
    int computeOffsetByParticleCount();
    int computeInnerOffsetAndBlockSum();
    int computeBlockOffset();
    int computeOffsetByBlockOffset();
    int assignParticleToCube();

    int searchNeighborFromCube();
    int clearNeighborCountPerParticle();
    int assignNeighborToParticle();

    int computeCurl();
    int particlePositionInit();
}
