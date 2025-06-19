#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "../common/common.hpp"

namespace simulator {
    // gui parameters
    extern int constraintProjectionIteration;
    extern float viscosityParameter;
    extern float vorticityParameter;
    extern int maxNeighborCount;
    extern common::real horizonMaxCoordinate;

    extern int uLeft;
    extern int uRight;
    extern int uUp;
    extern int uDown;
    extern int uFront;
    extern int uBack;
    extern float uDeltaVelocity;

    extern GLuint particlePositionSSBO;
    extern GLuint densitySSBO;

    #ifdef eGPU
    const unsigned int PARTICLE_COUNT_PER_EDGE_XZ = 48;
    #else
    const unsigned int PARTICLE_COUNT_PER_EDGE_XZ = 16;
    #endif
    const unsigned int PARTICLE_COUNT_PER_EDGE_Y = 128;
    const unsigned int PARTICLE_COUNT = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_Y;
    const common::real PARTICLE_RADIUS = 0.01;
    #ifdef eGPU
    const common::real HORIZON_MAX_COORDINATE = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_RADIUS * 5.0;
    #else
    const common::real HORIZON_MAX_COORDINATE = PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_RADIUS * 2.0 * 4.0;
    #endif
    const common::real MAX_HEIGHT = (PARTICLE_COUNT_PER_EDGE_Y + 60) * PARTICLE_RADIUS * 2.0;
    const common::real DELTA_TIME = 0.0016;
    const common::real DELTA_TIME_REVERSE = 1.0 / DELTA_TIME;

    const common::real KERNEL_RADIUS = PARTICLE_RADIUS * 4.0;
    const common::real REST_DENSITY = 1000.0;
    const common::real REST_DENSITY_REVERSE = 1.0 / REST_DENSITY;
    const common::real RELAXATION_PARAMETER = 1.0e-6;

    const glm::vec3 GRAVITY = glm::vec3(0.0, -9.8, 0.0);
    const common::real RESTITUTION = 1.0;
    const common::real FRICTION = 1.0;
    const common::real MASS = 6.4 * PARTICLE_RADIUS * PARTICLE_RADIUS * PARTICLE_RADIUS * REST_DENSITY;
    const common::real MASS_REVERSE = 1.0 / MASS;


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

    int computeCurl();
    int particlePositionInit();

    int manipulateVelocity();
}
