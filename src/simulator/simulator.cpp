#include "simulator.hpp"

#include <vector>
#include <iostream>

#include "../common/compute_shader.hpp"
#include "../common/performance_log.hpp"

namespace simulator {
    // gui parameters
    int constraintProjectionIteration = 4;
    float viscosityParameter = 0.005f;
    float vorticityParameter = 0.0f; 
    int maxNeighborCount = 256;
    common::real horizonMaxCoordinate = HORIZON_MAX_COORDINATE;

    int uLeft = 0;
    int uRight = 0;
    int uUp = 0;
    int uDown = 0;
    int uFront = 0;
    int uBack = 0;
    float uDeltaVelocity = 5.0f;

    // performance log
    const unsigned int QUERY_START_INDEX = 1;

    const GLuint CUBE_COUNT = GLuint(ceil(HORIZON_MAX_COORDINATE / KERNEL_RADIUS)) * GLuint(ceil(HORIZON_MAX_COORDINATE / KERNEL_RADIUS)) * GLuint(ceil(MAX_HEIGHT / KERNEL_RADIUS));
    const GLuint CUBE_COUNT_SQRT = GLuint(ceil(sqrt(CUBE_COUNT)));

    GLuint particlePositionSSBO;
    GLuint positionPredictSSBO;
    GLuint velocitySSBO;

    GLuint particleCountPerCubeSSBO;
    GLuint cubeOffsetSSBO;
    GLuint blockOffsetSSBO;
    GLuint particleIndexInCubeSSBO;

    GLuint neighborCountPerParticleSSBO;
    GLuint neighborIndexBufferSSBO;

    GLuint densitySSBO;
    GLuint constraintSSBO;
    GLuint constraintGradSquareSumSSBO;
    GLuint lambdaSSBO;

    GLuint deltaPositionSSBO;

    GLuint curlSSBO;
    GLuint curlXSSBO;
    GLuint curlYSSBO;
    GLuint curlZSSBO;

    ComputeShader applyExternalForcesCS;
    
    ComputeShader clearParticleCountPerCubeCS;
    ComputeShader computeParticleCountPerCubeCS;
    ComputeShader computeOffsetByParticleCountCS;
    ComputeShader computeInnerOffsetAndBlockSumCS;
    ComputeShader computeBlockOffsetCS;
    ComputeShader computeOffsetByBlockOffsetCS;
    ComputeShader assignParticleToCubeCS;

    ComputeShader searchNeighborFromCubeCS;

    ComputeShader computeDensityCS;
    ComputeShader computeConstraintCS;
    ComputeShader computeConstraintGradSquareSumCS;
    ComputeShader computeLambdaCS;

    ComputeShader handleBoundaryCollisionCS;
    ComputeShader computeDeltaPositionCS;
    ComputeShader adjustPositionPredictCS;
    ComputeShader updateVelocityByPositionCS;

    ComputeShader applyViscosityCS;

    ComputeShader computeCurlCS;
    ComputeShader applyVorticityConfinementCS;

    ComputeShader manipulateVelocityCS;

    int simulateInit() {
        glGenBuffers(1, &particlePositionSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePositionSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &positionPredictSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionPredictSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &velocitySSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitySSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &particleCountPerCubeSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleCountPerCubeSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &cubeOffsetSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cubeOffsetSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &particleIndexInCubeSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleIndexInCubeSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &blockOffsetSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockOffsetSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT_SQRT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &neighborCountPerParticleSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborCountPerParticleSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &neighborIndexBufferSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborIndexBufferSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * maxNeighborCount * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &densitySSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitySSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &constraintSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, constraintSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &constraintGradSquareSumSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, constraintGradSquareSumSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &lambdaSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lambdaSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &deltaPositionSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, deltaPositionSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &curlSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, curlSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePositionSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, positionPredictSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velocitySSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, particleCountPerCubeSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, cubeOffsetSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleIndexInCubeSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, blockOffsetSSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, neighborCountPerParticleSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, neighborIndexBufferSSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, densitySSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, constraintSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, constraintGradSquareSumSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, lambdaSSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, deltaPositionSSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, curlSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, curlXSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 18, curlYSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, curlZSSBO);


        particlePositionInit();

        applyExternalForcesCS = ComputeShader("src/simulator/shader/applyExternalForce.comp");
        handleBoundaryCollisionCS = ComputeShader("src/simulator/shader/handleBoundaryCollision.comp");

        clearParticleCountPerCubeCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/clearParticleCountPerCube.comp");
        computeParticleCountPerCubeCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/computeParticleCountPerCube.comp");
        computeOffsetByParticleCountCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/computeOffsetByParticleCount.comp");
        computeInnerOffsetAndBlockSumCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/computeInnerOffsetAndBlockSum.comp");
        computeBlockOffsetCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/computeBlockOffset.comp");
        computeOffsetByBlockOffsetCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/computeOffsetByBlockOffset.comp");
        assignParticleToCubeCS = ComputeShader("src/simulator/shader/searchNeighbor/divideCube/assignParticleToCube.comp");

            searchNeighborFromCubeCS = ComputeShader("src/simulator/shader/searchNeighbor/searchNeighborFromCube.comp");

        computeDensityCS = ComputeShader("src/simulator/shader/computeLambda/computeDensity.comp");
        computeConstraintCS = ComputeShader("src/simulator/shader/computeLambda/computeConstraint.comp");
        computeConstraintGradSquareSumCS = ComputeShader("src/simulator/shader/computeLambda/computeConstraintGradSquareSum.comp");
        computeLambdaCS = ComputeShader("src/simulator/shader/computeLambda/computeLambda.comp");
        
        computeDeltaPositionCS = ComputeShader("src/simulator/shader/computeDeltaPosition.comp");
        adjustPositionPredictCS = ComputeShader("src/simulator/shader/adjustPositionPredict.comp");
        updateVelocityByPositionCS = ComputeShader("src/simulator/shader/updateVelocityByPosition.comp");

        applyViscosityCS = ComputeShader("src/simulator/shader/applyViscosity.comp");

        computeCurlCS = ComputeShader("src/simulator/shader/applyVorticityConfinement/computeCurl.comp");
        applyVorticityConfinementCS = ComputeShader("src/simulator/shader/applyVorticityConfinement/applyVorticityConfinement.comp");

        manipulateVelocityCS = ComputeShader("src/simulator/shader/manipulateVelocity.comp");

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glFinish();

        return 0;
    }

    int simulate() {
        applyExternalForce();

        {
        common::queryTime(QUERY_START_INDEX + 0);
        }

        searchNeighbor();

        {
        common::queryTime(QUERY_START_INDEX + 1);
        }

        for (int i = 0; i < constraintProjectionIteration; i++) {
            computeLambda();
            computeDeltaPosition();
            handleBoundaryCollision();
            adjustPositionPredict();
        }

        {
        common::queryTime(QUERY_START_INDEX + 2);
        }

        updateVelocityByPosition();

        {
        common::queryTime(QUERY_START_INDEX + 3);
        }

        if (vorticityParameter > 0.0f)
            applyVorticityConfinement();

        {
        common::queryTime(QUERY_START_INDEX + 4);
        }

        if (viscosityParameter > 0.0f)
            applyViscosity();

        manipulateVelocity();

        {
        common::queryTime(QUERY_START_INDEX + 5);
        }

        handleBoundaryCollision();

        {
        common::queryTime(QUERY_START_INDEX + 6);
        }

        updateParticlePosition();


        return 0;
    }

    int simulateTerminate() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        for (GLuint i = 0; i < 20; i++) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, 0);
        }
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glDeleteBuffers(1, &particlePositionSSBO);
        glDeleteBuffers(1, &positionPredictSSBO);
        glDeleteBuffers(1, &velocitySSBO);
        glDeleteBuffers(1, &particleCountPerCubeSSBO);
        glDeleteBuffers(1, &cubeOffsetSSBO);
        glDeleteBuffers(1, &blockOffsetSSBO);
        glDeleteBuffers(1, &particleIndexInCubeSSBO);
        glDeleteBuffers(1, &neighborCountPerParticleSSBO);
        glDeleteBuffers(1, &neighborIndexBufferSSBO);
        glDeleteBuffers(1, &densitySSBO);
        glDeleteBuffers(1, &constraintSSBO);
        glDeleteBuffers(1, &constraintGradSquareSumSSBO);
        glDeleteBuffers(1, &lambdaSSBO);
        glDeleteBuffers(1, &deltaPositionSSBO);
        glDeleteBuffers(1, &curlSSBO);
        glDeleteBuffers(1, &curlXSSBO);
        glDeleteBuffers(1, &curlYSSBO);
        glDeleteBuffers(1, &curlZSSBO);

        glDeleteProgram(applyExternalForcesCS.ID);
        glDeleteProgram(clearParticleCountPerCubeCS.ID);
        glDeleteProgram(computeParticleCountPerCubeCS.ID);
        glDeleteProgram(computeOffsetByParticleCountCS.ID);
        glDeleteProgram(computeInnerOffsetAndBlockSumCS.ID);
        glDeleteProgram(computeBlockOffsetCS.ID);
        glDeleteProgram(computeOffsetByBlockOffsetCS.ID);
        glDeleteProgram(assignParticleToCubeCS.ID);
        glDeleteProgram(searchNeighborFromCubeCS.ID);
        glDeleteProgram(computeDensityCS.ID);
        glDeleteProgram(computeConstraintCS.ID);
        glDeleteProgram(computeConstraintGradSquareSumCS.ID);
        glDeleteProgram(computeLambdaCS.ID);
        glDeleteProgram(handleBoundaryCollisionCS.ID);
        glDeleteProgram(computeDeltaPositionCS.ID);
        glDeleteProgram(adjustPositionPredictCS.ID);
        glDeleteProgram(updateVelocityByPositionCS.ID);
        glDeleteProgram(applyViscosityCS.ID);
        glDeleteProgram(computeCurlCS.ID);
        glDeleteProgram(applyVorticityConfinementCS.ID);

        glFinish();

        return 0;
    }


    int applyExternalForce() {
        applyExternalForcesCS.use();
        applyExternalForcesCS.setVec3("GRAVITY", GRAVITY);
        applyExternalForcesCS.setFloat("DELTA_TIME", static_cast<float>(DELTA_TIME));
        applyExternalForcesCS.setFloat("MASS_REVERSE", static_cast<float>(MASS_REVERSE));
        applyExternalForcesCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        applyExternalForcesCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
    
    int searchNeighbor() {
        divideCube();
        searchNeighborFromCube();

        return 0;
    }

    int computeLambda() {
        computeDensity();
        computeConstraint();
        computeConstraintGradSquareSum();

        computeLambdaCS.use();
        computeLambdaCS.setFloat("RELAXATION_PARAMETER", static_cast<float>(RELAXATION_PARAMETER));
        computeLambdaCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeLambdaCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeDeltaPosition() {
        computeDeltaPositionCS.use();
        computeDeltaPositionCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeDeltaPositionCS.setFloat("MASS", static_cast<float>(MASS));
        computeDeltaPositionCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeDeltaPositionCS.setFloat("PI", static_cast<float>(common::PI));
        computeDeltaPositionCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        computeDeltaPositionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeDeltaPositionCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int handleBoundaryCollision() {
        handleBoundaryCollisionCS.use();
        handleBoundaryCollisionCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(horizonMaxCoordinate));
        handleBoundaryCollisionCS.setFloat("RESTITUTION", static_cast<float>(RESTITUTION));
        handleBoundaryCollisionCS.setFloat("FRICTION", static_cast<float>(FRICTION));
        handleBoundaryCollisionCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));
        handleBoundaryCollisionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        handleBoundaryCollisionCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
   
    int adjustPositionPredict() {
        adjustPositionPredictCS.use();
        adjustPositionPredictCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        adjustPositionPredictCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int updateVelocityByPosition() {
        updateVelocityByPositionCS.use();
        updateVelocityByPositionCS.setFloat("DELTA_TIME_REVERSE", static_cast<float>(DELTA_TIME_REVERSE));
        updateVelocityByPositionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        updateVelocityByPositionCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int applyViscosity() {
        applyViscosityCS.use();
        applyViscosityCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        applyViscosityCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        applyViscosityCS.setFloat("VISCOSITY_PARAMETER", static_cast<float>(viscosityParameter));
        applyViscosityCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        applyViscosityCS.setFloat("PI", static_cast<float>(common::PI));
        applyViscosityCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        applyViscosityCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int applyVorticityConfinement() {
        computeCurl();

        applyVorticityConfinementCS.use();
        applyVorticityConfinementCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        applyVorticityConfinementCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        applyVorticityConfinementCS.setFloat("PI", static_cast<float>(common::PI));
        applyVorticityConfinementCS.setFloat("VORTICITY_PARAMETER", static_cast<float>(vorticityParameter));
        applyVorticityConfinementCS.setFloat("MASS_REVERSE", static_cast<float>(MASS_REVERSE));
        applyVorticityConfinementCS.setFloat("DELTA_TIME", static_cast<float>(DELTA_TIME));
        applyVorticityConfinementCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        applyVorticityConfinementCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int updateParticlePosition() {
        glBindBuffer(GL_COPY_READ_BUFFER, positionPredictSSBO);
        glBindBuffer(GL_COPY_WRITE_BUFFER, particlePositionSSBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, PARTICLE_COUNT * sizeof(glm::vec4));
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        return 0;
    }


    int divideCube() {
        clearParticleCountPerCube();
        computeParticleCountPerCube();

        // computeOffsetByParticleCount();

        computeInnerOffsetAndBlockSum();
        computeBlockOffset();
        computeOffsetByBlockOffset();

        assignParticleToCube();

        return 0;
    }

    int clearParticleCountPerCube() {
        clearParticleCountPerCubeCS.use();
        clearParticleCountPerCubeCS.setUint("CUBE_COUNT", CUBE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        clearParticleCountPerCubeCS.dispatchCompute(CUBE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeParticleCountPerCube() {
        computeParticleCountPerCubeCS.use();
        computeParticleCountPerCubeCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeParticleCountPerCubeCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        computeParticleCountPerCubeCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));
        computeParticleCountPerCubeCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeParticleCountPerCubeCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeOffsetByParticleCount() {
        computeOffsetByParticleCountCS.use();
        computeOffsetByParticleCountCS.setUint("CUBE_COUNT", CUBE_COUNT);

        // glDispatchCompute(1, 1, 1);
        computeOffsetByParticleCountCS.dispatchCompute(1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeInnerOffsetAndBlockSum() {
        computeInnerOffsetAndBlockSumCS.use();
        computeInnerOffsetAndBlockSumCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);
        computeInnerOffsetAndBlockSumCS.setUint("CUBE_COUNT", CUBE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT_SQRT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeInnerOffsetAndBlockSumCS.dispatchCompute(CUBE_COUNT_SQRT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }   

    int computeBlockOffset() {
        computeBlockOffsetCS.use();
        computeBlockOffsetCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);

        // glDispatchCompute(1, 1, 1);
        computeBlockOffsetCS.dispatchCompute(1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeOffsetByBlockOffset() {
        computeOffsetByBlockOffsetCS.use();
        computeOffsetByBlockOffsetCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);
        computeOffsetByBlockOffsetCS.setUint("CUBE_COUNT", CUBE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT_SQRT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeOffsetByBlockOffsetCS.dispatchCompute(CUBE_COUNT_SQRT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int assignParticleToCube() {
        assignParticleToCubeCS.use();
        assignParticleToCubeCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        assignParticleToCubeCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);
        assignParticleToCubeCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        assignParticleToCubeCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        assignParticleToCubeCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }


    int searchNeighborFromCube() {
        searchNeighborFromCubeCS.use();
        searchNeighborFromCubeCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        searchNeighborFromCubeCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);
        searchNeighborFromCubeCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        searchNeighborFromCubeCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        searchNeighborFromCubeCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        searchNeighborFromCubeCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }


    int computeDensity() {
        computeDensityCS.use();
        computeDensityCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeDensityCS.setFloat("MASS", static_cast<float>(MASS));
        computeDensityCS.setFloat("PI", static_cast<float>(common::PI));
        computeDensityCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        computeDensityCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeDensityCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeConstraint() {
        computeConstraintCS.use();
        computeConstraintCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeConstraintCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeConstraintCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeConstraintGradSquareSum() {
        computeConstraintGradSquareSumCS.use();
        computeConstraintGradSquareSumCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeConstraintGradSquareSumCS.setFloat("MASS", static_cast<float>(MASS));
        computeConstraintGradSquareSumCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeConstraintGradSquareSumCS.setFloat("PI", static_cast<float>(common::PI));
        computeConstraintGradSquareSumCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        computeConstraintGradSquareSumCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeConstraintGradSquareSumCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }


    int computeCurl() {
        computeCurlCS.use();
        computeCurlCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeCurlCS.setUint("MAX_NEIGHBOR_COUNT", maxNeighborCount);
        computeCurlCS.setFloat("PI", static_cast<float>(common::PI));
        computeCurlCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        // glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        computeCurlCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
   
    int particlePositionInit() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePositionSSBO);

        std::vector<glm::vec4> particlePositionVector(PARTICLE_COUNT);
        const common::real DIAMETER = PARTICLE_RADIUS * 2.0;

        // Tow Dam Break
        unsigned int index = 0;
        common::real x = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
        for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
            common::real y = 100.0 * DIAMETER;
            for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y / 2; j++) {
                common::real z = 0.5 * HORIZON_MAX_COORDINATE - 5.0 * DIAMETER;
                for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
                    particlePositionVector[index++] = glm::vec4(x, y, z, 0.0);
                    z -= DIAMETER;
                }
                y += DIAMETER;
            }
            x += DIAMETER;
        }
        x = 0.5 * HORIZON_MAX_COORDINATE - 5.0 * DIAMETER;
        for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
            common::real y = 100.0 * DIAMETER;
            for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y / 2; j++) {
                common::real z = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
                for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
                    particlePositionVector[index++] = glm::vec4(x, y, z, 0.0);
                    z += DIAMETER;
                }
                y += DIAMETER;
            }
            x -= DIAMETER;
        }

        // One Dam Break
        // unsigned int index = 0;
        // common::real x = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
        // for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
        //     common::real y = 50.0 * DIAMETER;
        //     for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y; j++) {
        //         common::real z = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
        //         for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
        //             particlePositionVector[index++] = glm::vec4(x, y, z, 0.0);
        //             z += DIAMETER;
        //         }
        //         y += DIAMETER;
        //     }
        //     x += DIAMETER;
        // }

        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), particlePositionVector.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return 0;
    }

    int manipulateVelocity() {
        manipulateVelocityCS.use();
        manipulateVelocityCS.setUint("uParticleCount", PARTICLE_COUNT);
        manipulateVelocityCS.setFloat("uDeltaVelocity", uDeltaVelocity);
        
        manipulateVelocityCS.setInt("uLeft", uLeft);
        manipulateVelocityCS.setInt("uRight", uRight);
        manipulateVelocityCS.setInt("uUp", uUp);
        manipulateVelocityCS.setInt("uDown", uDown);
        manipulateVelocityCS.setInt("uFront", uFront);
        manipulateVelocityCS.setInt("uBack", uBack);
        
        manipulateVelocityCS.dispatchCompute(PARTICLE_COUNT);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
  
}
