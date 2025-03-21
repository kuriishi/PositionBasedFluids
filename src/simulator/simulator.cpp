#include "simulator.hpp"

#include <vector>
#include <iostream>

#include "compute_shader.hpp"

using glm::vec4;

using std::vector;
using std::cout;
using std::endl;

using common::PI;

namespace simulator {
    const GLuint INVOCATION_PER_WORKGROUP = 256;
    const GLuint MAX_NEIGHBOR_COUNT = 64;
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
    GLuint particleIndexInNeighborSSBO;

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

    ComputeShader clearNeighborCountPerParticleCS;
    ComputeShader assignNeighborToParticleCS;

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

    int simulateInit() {
        glGenBuffers(1, &particlePositionSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePositionSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &positionPredictSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionPredictSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &velocitySSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitySSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &particleCountPerCubeSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleCountPerCubeSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &cubeOffsetSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cubeOffsetSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &particleIndexInCubeSSBO);
        glGenBuffers(1, &blockOffsetSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockOffsetSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, CUBE_COUNT_SQRT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleIndexInCubeSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &neighborCountPerParticleSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborCountPerParticleSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &particleIndexInNeighborSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleIndexInNeighborSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * MAX_NEIGHBOR_COUNT * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

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
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &curlSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, curlSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePositionSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, positionPredictSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velocitySSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, particleCountPerCubeSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, cubeOffsetSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleIndexInCubeSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, blockOffsetSSBO);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, neighborCountPerParticleSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, particleIndexInNeighborSSBO);

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

        clearNeighborCountPerParticleCS = ComputeShader("src/simulator/shader/searchNeighbor/searchNeighborFromCube/clearNeighborCountPerParticle.comp");
        assignNeighborToParticleCS = ComputeShader("src/simulator/shader/searchNeighbor/searchNeighborFromCube/assignNeighborToParticle.comp");

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

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return 0;
    }

    int simulate() {
        applyExternalForce();

        searchNeighbor();
        for (unsigned int i = 0; i < ITERATION; i++) {
            computeLambda();
            computeDeltaPosition();
            handleBoundaryCollision();
            adjustPositionPredict();
        }
        updateVelocityByPosition();

        applyViscosity();
        applyVorticityConfinement();

        updateParticlePosition();

        return 0;
    }

    int simulateTerminate() {
        
        return 0;
    }


    int applyExternalForce() {
        applyExternalForcesCS.use();
        applyExternalForcesCS.setVec3("GRAVITY", GRAVITY);
        applyExternalForcesCS.setFloat("DELTA_TIME", static_cast<float>(DELTA_TIME));
        applyExternalForcesCS.setFloat("MASS_REVERSE", static_cast<float>(MASS_REVERSE));
        applyExternalForcesCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
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

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeDeltaPosition() {
        computeDeltaPositionCS.use();
        computeDeltaPositionCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeDeltaPositionCS.setFloat("MASS", static_cast<float>(MASS));
        computeDeltaPositionCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeDeltaPositionCS.setFloat("PI", static_cast<float>(PI));
        computeDeltaPositionCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        computeDeltaPositionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int handleBoundaryCollision() {
        handleBoundaryCollisionCS.use();
        handleBoundaryCollisionCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        handleBoundaryCollisionCS.setFloat("RESTITUTION", static_cast<float>(RESTITUTION));
        handleBoundaryCollisionCS.setFloat("FRICTION", static_cast<float>(FRICTION));
        handleBoundaryCollisionCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));
        handleBoundaryCollisionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
   
    int adjustPositionPredict() {
        adjustPositionPredictCS.use();
        adjustPositionPredictCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int updateVelocityByPosition() {
        updateVelocityByPositionCS.use();
        updateVelocityByPositionCS.setFloat("DELTA_TIME_REVERSE", static_cast<float>(DELTA_TIME_REVERSE));
        updateVelocityByPositionCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int applyViscosity() {
        applyViscosityCS.use();
        applyViscosityCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        applyViscosityCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        applyViscosityCS.setFloat("VISCOSITY_PARAMETER", static_cast<float>(VISCOSITY_PARAMETER));
        applyViscosityCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        applyViscosityCS.setFloat("PI", static_cast<float>(PI));
        applyViscosityCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int applyVorticityConfinement() {
        computeCurl();

        applyVorticityConfinementCS.use();
        applyVorticityConfinementCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        applyVorticityConfinementCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        applyVorticityConfinementCS.setFloat("PI", static_cast<float>(PI));
        applyVorticityConfinementCS.setFloat("VORTICITY_PARAMETER", static_cast<float>(VORTICITY_PARAMETER));
        applyVorticityConfinementCS.setFloat("MASS_REVERSE", static_cast<float>(MASS_REVERSE));
        applyVorticityConfinementCS.setFloat("DELTA_TIME", static_cast<float>(DELTA_TIME));
        applyVorticityConfinementCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int updateParticlePosition() {
        glBindBuffer(GL_COPY_READ_BUFFER, positionPredictSSBO);
        glBindBuffer(GL_COPY_WRITE_BUFFER, particlePositionSSBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, PARTICLE_COUNT * sizeof(vec4));
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

        glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeParticleCountPerCube() {
        computeParticleCountPerCubeCS.use();
        computeParticleCountPerCubeCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeParticleCountPerCubeCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        computeParticleCountPerCubeCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));
        computeParticleCountPerCubeCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeOffsetByParticleCount() {
        computeOffsetByParticleCountCS.use();
        computeOffsetByParticleCountCS.setUint("CUBE_COUNT", CUBE_COUNT);

        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeInnerOffsetAndBlockSum() {
        computeInnerOffsetAndBlockSumCS.use();
        computeInnerOffsetAndBlockSumCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);
        computeInnerOffsetAndBlockSumCS.setUint("CUBE_COUNT", CUBE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT_SQRT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }   

    int computeBlockOffset() {
        computeBlockOffsetCS.use();
        computeBlockOffsetCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);

        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeOffsetByBlockOffset() {
        computeOffsetByBlockOffsetCS.use();
        computeOffsetByBlockOffsetCS.setUint("CUBE_COUNT_SQRT", CUBE_COUNT_SQRT);
        computeOffsetByBlockOffsetCS.setUint("CUBE_COUNT", CUBE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(CUBE_COUNT_SQRT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int assignParticleToCube() {
        assignParticleToCubeCS.use();
        assignParticleToCubeCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        assignParticleToCubeCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);
        assignParticleToCubeCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        assignParticleToCubeCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }


    int searchNeighborFromCube() {
        clearNeighborCountPerParticle();
        assignNeighborToParticle();

        return 0;
    }

    int clearNeighborCountPerParticle() {
        clearNeighborCountPerParticleCS.use();
        clearNeighborCountPerParticleCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int assignNeighborToParticle() {
        assignNeighborToParticleCS.use();
        assignNeighborToParticleCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        assignNeighborToParticleCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);
        assignNeighborToParticleCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        assignNeighborToParticleCS.setFloat("HORIZON_MAX_COORDINATE", static_cast<float>(HORIZON_MAX_COORDINATE));
        assignNeighborToParticleCS.setFloat("MAX_HEIGHT", static_cast<float>(MAX_HEIGHT));

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
  

    int computeDensity() {
        computeDensityCS.use();
        computeDensityCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeDensityCS.setFloat("MASS", static_cast<float>(MASS));
        computeDensityCS.setFloat("PI", static_cast<float>(PI));
        computeDensityCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        computeDensityCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeConstraint() {
        computeConstraintCS.use();
        computeConstraintCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeConstraintCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }

    int computeConstraintGradSquareSum() {
        computeConstraintGradSquareSumCS.use();
        computeConstraintGradSquareSumCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeConstraintGradSquareSumCS.setFloat("MASS", static_cast<float>(MASS));
        computeConstraintGradSquareSumCS.setFloat("REST_DENSITY_REVERSE", static_cast<float>(REST_DENSITY_REVERSE));
        computeConstraintGradSquareSumCS.setFloat("PI", static_cast<float>(PI));
        computeConstraintGradSquareSumCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        computeConstraintGradSquareSumCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }


    int computeCurl() {
        computeCurlCS.use();
        computeCurlCS.setFloat("KERNEL_RADIUS", static_cast<float>(KERNEL_RADIUS));
        computeCurlCS.setUint("MAX_NEIGHBOR_COUNT", MAX_NEIGHBOR_COUNT);
        computeCurlCS.setFloat("PI", static_cast<float>(PI));
        computeCurlCS.setUint("PARTICLE_COUNT", PARTICLE_COUNT);

        glDispatchCompute(GLuint(ceil(static_cast<double>(PARTICLE_COUNT) / INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        return 0;
    }
   
    int particlePositionInit() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePositionSSBO);

        vector<vec4> particlePositionsVector(PARTICLE_COUNT);
        const real DIAMETER = PARTICLE_RADIUS * 2.0;

        unsigned int index = 0;
        real x = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
        for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
            real y = 20.0 * DIAMETER;
            for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y / 2; j++) {
                real z = 0.5 * HORIZON_MAX_COORDINATE - 5.0 * DIAMETER;
                // real z = 5.0 * DIAMETER;
                for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
                    particlePositionsVector[index++] = vec4(x, y, z, 0.0);
                    z -= DIAMETER;
                }
                y += DIAMETER;
            }
            x += DIAMETER;
        }
        x = 0.5 * HORIZON_MAX_COORDINATE - 5.0 * DIAMETER;
        for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
            real y = 20.0 * DIAMETER;
            for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y / 2; j++) {
                real z = -0.5 * HORIZON_MAX_COORDINATE + 5.0 * DIAMETER;
                // real z = -5.0 * DIAMETER;
                for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
                    particlePositionsVector[index++] = vec4(x, y, z, 0.0);
                    z += DIAMETER;
                }
                y += DIAMETER;
            }
            x -= DIAMETER;
        }

        glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * sizeof(vec4), particlePositionsVector.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return 0;
    }
  
}
