#include "simulator.hpp"

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <execution>

#include "../renderer/window.hpp"
#include "kernel.hpp"

using glm::cross;
using glm::isnan;
using glm::length;

using std::cout;
using std::endl;
using std::clamp;
using std::floor;
using std::unordered_map;
using std::for_each;
using std::execution::par_unseq;
using std::iota;
using std::max;
using std::min;

using simulator::kernel::Poly6;
using simulator::kernel::SpikyGradient;

// hash ivec3 for computeNeighbors with unordered_map
namespace std {
    template<>
    struct hash<glm::ivec3> {
        size_t operator()(const glm::ivec3& v) const {
            return ((hash<int>()(v.x) ^ (hash<int>()(v.y) << 1)) >> 1) ^ (hash<int>()(v.z) << 1);
        }
    };
}

namespace simulator {
    // particle properties
    vector<vec3> particlePositions;
    vector<vec3> positionPredict;
    vector<vec3> velocity;
    vector<vec3> force;
    vector<real> density;
    vector<real> constraint;
    vector<real> lambda;
    vector<vec3> deltaPosition;
    vector<vec3> curl;
    vector<vec3> curlX;
    vector<vec3> curlY;
    vector<vec3> curlZ;

    unordered_map<ivec3, vector<unsigned int>, std::hash<ivec3>, std::equal_to<ivec3>> spaceCube;
    vector<vector<unsigned int>> neighbors;
    
    vector<unsigned int> indices;

    int simulateInit() {
        particlePositions.resize(PARTICLE_COUNT);
        positionPredict.resize(PARTICLE_COUNT);
        velocity.resize(PARTICLE_COUNT);
        force.resize(PARTICLE_COUNT);
        density.resize(PARTICLE_COUNT);
        constraint.resize(PARTICLE_COUNT);
        lambda.resize(PARTICLE_COUNT);
        deltaPosition.resize(PARTICLE_COUNT);
        indices.resize(PARTICLE_COUNT);
        curl.resize(PARTICLE_COUNT);
        curlX.resize(PARTICLE_COUNT);
        curlY.resize(PARTICLE_COUNT);
        curlZ.resize(PARTICLE_COUNT);
        iota(indices.begin(), indices.end(), 0);

        const real DIAMETER = PARTICLE_RADIUS * 2.0;

        real x = -1.0 * PARTICLE_COUNT_PER_EDGE_XZ * DIAMETER + DIAMETER;
        for (unsigned int i = 0; i < PARTICLE_COUNT_PER_EDGE_XZ; i++) {
            real y = 10.0 * DIAMETER;
            for (unsigned int j = 0; j < PARTICLE_COUNT_PER_EDGE_Y; j++) {
                real z = -1.0 * PARTICLE_COUNT_PER_EDGE_XZ * DIAMETER + DIAMETER;
                for (unsigned int k = 0; k < PARTICLE_COUNT_PER_EDGE_XZ; k++) {
                    particlePositions[i * PARTICLE_COUNT_PER_EDGE_XZ * PARTICLE_COUNT_PER_EDGE_Y + j * PARTICLE_COUNT_PER_EDGE_XZ + k] = vec3(x, y, z);
                    z += DIAMETER;
                }
                y += DIAMETER;
            }
            x += DIAMETER;
        }

        return 0;
    }

    int simulate() {
        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            velocity[i] += GRAVITY * static_cast<float>(DELTA_TIME * MASS_REVERSE);
            positionPredict[i] = particlePositions[i] + velocity[i] * static_cast<float>(DELTA_TIME);
        });

        computeNeighbors();
        for (unsigned int iteration = 0; iteration < ITERATION; iteration++) {
            computeDensity();
            for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
                lambda[i] = computeLambda(i);
            });

            for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
                deltaPosition[i] = computeDeltaPosition(i);
                boundaryCollisionHandling(i);
            });

            for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
                positionPredict[i] += deltaPosition[i];
            });
        }

        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            velocity[i] = (positionPredict[i] - particlePositions[i]) * static_cast<float>(DELTA_TIME_REVERSE);
        });

        applyViscosity();
        applyVorticityConfinement();

        particlePositions = positionPredict;

        return 0;
    }

    int simulateTerminate() {
        particlePositions.clear();
        positionPredict.clear();
        velocity.clear();
        force.clear();
        density.clear();
        constraint.clear();
        lambda.clear();
        deltaPosition.clear();
        spaceCube.clear();
        neighbors.clear();
        curl.clear();
        curlX.clear();
        curlY.clear();
        curlZ.clear();
        indices.clear();

        return 0;
    }

    int computeNeighbors() {
        spaceCube.clear();
        neighbors.clear();
        neighbors.resize(PARTICLE_COUNT);

        for (unsigned int i = 0; i < PARTICLE_COUNT; i++) {
            ivec3 cubeIndex = getCubeIndex(positionPredict[i]);
            spaceCube[cubeIndex].push_back(i);
        }

        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            ivec3 cubeIndex = getCubeIndex(positionPredict[i]);
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        ivec3 neighborCubeIndex = cubeIndex + ivec3(dx, dy, dz);
                        if (spaceCube.find(neighborCubeIndex) != spaceCube.end()) {
                            for (unsigned int neighbor : spaceCube[neighborCubeIndex]) {
                                if (neighbor != i) {
                                    real distance = length(positionPredict[i] - positionPredict[neighbor]);
                                    if (distance <= KERNEL_RADIUS) {
                                        neighbors[i].push_back(neighbor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        });

        return 0;
    }

    real computeLambda(const unsigned int index) {
        real constraint = computeConstraint(index);
        real constraintGradSquareSum = computeConstraintGradSquareSum(index);
        real lambda = -constraint / (constraintGradSquareSum + RELAXATION_PARAMETER);

        return lambda;
    }

    vec3 computeDeltaPosition(const unsigned int index) {
        vec3 deltaPosition = vec3(0.0);
        for (unsigned int j: neighbors[index]) {
            deltaPosition += static_cast<float>(lambda[index] + lambda[j]) * SpikyGradient(positionPredict[index] - positionPredict[j], KERNEL_RADIUS);
        }
        deltaPosition *= MASS * REST_DENSITY_REVERSE;

        return deltaPosition;
    }

    int boundaryCollisionHandling(const unsigned int index) {
        if (positionPredict[index].x <= -0.5 * HORIZON_MAX_COORDINATE) {
            positionPredict[index].x = static_cast<float>(-0.5 * HORIZON_MAX_COORDINATE);
            if (velocity[index].x < 0.0) {
                velocity[index].x *= -static_cast<float>(RESTITUTION);
                velocity[index].y *= static_cast<float>(FRICTION);
                velocity[index].z *= static_cast<float>(FRICTION);
            }
        }
        if (positionPredict[index].x >= 0.5 * HORIZON_MAX_COORDINATE) {
            positionPredict[index].x = static_cast<float>(0.5 * HORIZON_MAX_COORDINATE);
            if (velocity[index].x > 0.0) {
                velocity[index].x *= -static_cast<float>(RESTITUTION);
                velocity[index].y *= static_cast<float>(FRICTION);
                velocity[index].z *= static_cast<float>(FRICTION);
            }
        }
        if (positionPredict[index].y <= 0.0) {
            positionPredict[index].y = static_cast<float>(0.0);
            if (velocity[index].y < 0.0) {
                velocity[index].y *= -static_cast<float>(RESTITUTION);
                velocity[index].x *= static_cast<float>(FRICTION);
                velocity[index].z *= static_cast<float>(FRICTION);
            }
        }
        if (positionPredict[index].z <= -0.5 * HORIZON_MAX_COORDINATE) {
            positionPredict[index].z = static_cast<float>(-0.5 * HORIZON_MAX_COORDINATE);
            if (velocity[index].z < 0.0) {
                velocity[index].z *= -static_cast<float>(RESTITUTION);
                velocity[index].x *= static_cast<float>(FRICTION);
                velocity[index].y *= static_cast<float>(FRICTION);
            }
        }
        if (positionPredict[index].z >= 0.5 * HORIZON_MAX_COORDINATE) {
            positionPredict[index].z = static_cast<float>(0.5 * HORIZON_MAX_COORDINATE);
            if (velocity[index].z > 0.0) {
                velocity[index].z *= -static_cast<float>(RESTITUTION);
                velocity[index].x *= static_cast<float>(FRICTION);
                velocity[index].y *= static_cast<float>(FRICTION);
            }
        }

        return 0;
    }

    int applyVorticityConfinement() {
        computeCurl();
        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            // vec3 eta = vec3(0.0);
            // for (unsigned int j: neighbors[i]) {
            //     eta += vorticity[j].length() * static_cast<float>(Spiky(positionPredict[i] - positionPredict[j], KERNEL_RADIUS));
            // }
            // eta *= REST_DENSITY_REVERSE;
            // vec3 n = normalize(eta);
            // vec3 force = static_cast<float>(VORTICITY_PARAMETER) * cross(n, vorticity[i]);
            // velocity[i] += force * static_cast<float>(DELTA_TIME);
            if (!isnan(curl[i].x) && !isnan(curl[i].y) && !isnan(curl[i].z)) {
                real curlLength = length(curl[i]);
                vec3 n;
                n.x = length(curlX[i]) - static_cast<float>(curlLength);
                n.y = length(curlY[i]) - static_cast<float>(curlLength);
                n.z = length(curlZ[i]) - static_cast<float>(curlLength);
                n = normalize(n);
                if (!isnan(n.x) && !isnan(n.y) && !isnan(n.z)) {
                    vec3 force = static_cast<float>(VORTICITY_PARAMETER) * cross(n, curl[i]);
                    velocity[i] += force * static_cast<float>(DELTA_TIME * MASS_REVERSE);
                }
            }
        });

        return 0;
    }

    int applyViscosity() {
        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            vec3 deltaVelocity = vec3(0.0);
            for (unsigned int j: neighbors[i]) {
                deltaVelocity -= (velocity[i] - velocity[j]) * static_cast<float>(Poly6(positionPredict[i] - positionPredict[j], KERNEL_RADIUS));
            }
            velocity[i] += static_cast<float>(VISCOSITY_PARAMETER * REST_DENSITY_REVERSE) * deltaVelocity;
            // velocity[i] = static_cast<float>(1.0 - VISCOSITY_PARAMETER) * velocity[i] + static_cast<float>(VISCOSITY_PARAMETER * REST_DENSITY_REVERSE) * deltaVelocity;
        });

        return 0;
    }

    ivec3 getCubeIndex(const vec3& position) {
        return ivec3(
            static_cast<int>(floor(position.x / KERNEL_RADIUS)),
            static_cast<int>(floor(position.y / KERNEL_RADIUS)),
            static_cast<int>(floor(position.z / KERNEL_RADIUS))
        );
    }

    real computeConstraint(const unsigned int index) {
        real constraint = density[index] * REST_DENSITY_REVERSE - 1.0;
        constraint = max(constraint, 0.0);

        return constraint;
    }

    real computeConstraintGradSquareSum(const unsigned int index) {
        real constraintGradSquareSum = 0.0;
        vec3 constraintGrad_i = vec3(0.0);
        for (unsigned int j: neighbors[index]) {
            vec3 constraintGrad_j = vec3(0.0);
            // if (j == index) {
            //     for (unsigned int k: neighbors[index]) {
            //         constraintGrad += SpikyGradient(positionPredict[index] - positionPredict[k], KERNEL_RADIUS);
            //     }
            // }
            // else {
            //     constraintGrad = -SpikyGradient(positionPredict[index] - positionPredict[j], KERNEL_RADIUS);
            // }
            constraintGrad_j = SpikyGradient(positionPredict[index] - positionPredict[j], KERNEL_RADIUS);
            constraintGrad_j *= MASS * REST_DENSITY_REVERSE;
            constraintGradSquareSum += dot(constraintGrad_j, constraintGrad_j);
            constraintGrad_i += constraintGrad_j;
        }
        constraintGradSquareSum += dot(constraintGrad_i, constraintGrad_i);

        return constraintGradSquareSum;
    }

    int computeDensity() {
        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            density[i] = Poly6(vec3(0.0), KERNEL_RADIUS);
            for (unsigned int j: neighbors[i]) {
                density[i] += Poly6(positionPredict[i] - positionPredict[j], KERNEL_RADIUS);
            }
            density[i] *= MASS;
        });

        return 0;
    }

    int computeCurl() {
        for_each(par_unseq, indices.begin(), indices.end(), [](unsigned int i) {
            curl[i] = vec3(0.0);
            curlX[i] = vec3(0.0);
            curlY[i] = vec3(0.0);
            curlZ[i] = vec3(0.0);
            for (unsigned int j: neighbors[i]) {
                vec3 v_ji = velocity[j] - velocity[i];
                vec3 p_ij = positionPredict[i] - positionPredict[j];
                curl[i] += cross(v_ji, SpikyGradient(p_ij, KERNEL_RADIUS));
                curlX[i] += cross(v_ji, SpikyGradient(p_ij + vec3(0.01, 0.0, 0.0), KERNEL_RADIUS));
                curlY[i] += cross(v_ji, SpikyGradient(p_ij + vec3(0.0, 0.01, 0.0), KERNEL_RADIUS));
                curlZ[i] += cross(v_ji, SpikyGradient(p_ij + vec3(0.0, 0.0, 0.01), KERNEL_RADIUS));
            }
        });

        return 0;
    }

}
