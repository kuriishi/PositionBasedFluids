#include "renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

#include <iostream>
#include <cmath>

#include "../common/common.hpp"
#include "shader.hpp"
#include "window.hpp"
#include "../simulator/simulator.hpp"

using glm::mat3;
using glm::mat4;
using glm::translate;
using glm::scale;
using glm::perspective;
using glm::radians;

using std::cout;
using std::endl;
using std::sin;
using std::cos;

using common::PI;
using simulator::particlePositions;
using simulator::PARTICLE_COUNT;
using simulator::PARTICLE_RADIUS;

namespace renderer {
    // skybox
    vector<float> skyboxVertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    vector<string> skyBoxFaces = {
        "resource/skybox/right.jpg",
        "resource/skybox/left.jpg",
        "resource/skybox/top.jpg",
        "resource/skybox/bottom.jpg",
        "resource/skybox/front.jpg",
        "resource/skybox/back.jpg"
    };
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int skyboxTexture;
    Shader skyboxShader;

    // sphere
    vector<vec3> sphereVertices;
    vector<unsigned int> sphereIndices;
    unsigned int sphereIndexCount;
    const unsigned int SPHERE_SEGMENTS = 64;
    unsigned int sphereVBO;
    unsigned int sphereEBO;

    // particles
    unsigned int particlesInstanceVAO;
    unsigned int particlesInstanceVBO;
    Shader particlesShader;
    
    int renderInitSkybox() {
        glEnable(GL_DEPTH_TEST);
        skyboxShader = Shader("src/renderer/shader/skybox.vert", "src/renderer/shader/skybox.frag");

        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        stbi_set_flip_vertically_on_load(false);
        skyboxTexture = loadCubemap(skyBoxFaces);
        skyboxShader.use();
        skyboxShader.setInt("skyboxTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glBindVertexArray(0);

        return 0;
    }

    int renderSkybox() {
        glDepthFunc(GL_LEQUAL);

        skyboxShader.use();

        mat4 view = mat4(mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);

        mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
    
        return 0;
    }

    int renderTerminateSkybox() {
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);
        glDeleteTextures(1, &skyboxTexture);
        glDeleteProgram(skyboxShader.ID);

        skyboxVertices.clear();
        skyBoxFaces.clear();

        return 0;
    }

    int renderInitParticles() {
        generateSphere(SPHERE_SEGMENTS);

        glEnable(GL_DEPTH_TEST);
        particlesShader = Shader("src/renderer/shader/particles.vert", "src/renderer/shader/particles.frag");

        glGenVertexArrays(1, &particlesInstanceVAO);
        glGenBuffers(1, &particlesInstanceVBO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(particlesInstanceVAO);

        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(vec3), &sphereVertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, particlesInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(vec3), &particlePositions[0], GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
        glEnableVertexAttribArray(2);

        glVertexAttribDivisor(2, 1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return 0;
    }

    int renderParticles() {
        particlesShader.use();

        // 更新粒子位置数据
        glBindBuffer(GL_ARRAY_BUFFER, particlesInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(vec3), &particlePositions[0], GL_DYNAMIC_DRAW);

        mat4 model = mat4(1.0f);
        model = scale(model, vec3(static_cast<float>(PARTICLE_RADIUS)));
        particlesShader.setMat4("model", model);

        mat4 view = camera.GetViewMatrix();
        particlesShader.setMat4("view", view);

        mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        particlesShader.setMat4("projection", projection);

        mat3 normalMatrix = mat3(transpose(inverse(model)));
        particlesShader.setMat3("normalMatrix", normalMatrix);

        vec3 lightDir = vec3(1.0f, 1.0f, 0.5f);
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        float ambient = 0.1f;
        float diffuse = 1.0f;
        float specular = 0.5f;
        float shininess = 64.0f;
        particlesShader.setVec3("light.direction", lightDir);
        particlesShader.setVec3("light.color", lightColor);
        particlesShader.setFloat("light.ambient", ambient);
        particlesShader.setFloat("light.diffuse", diffuse);
        particlesShader.setFloat("light.specular", specular);
        particlesShader.setFloat("light.shininess", shininess);

        vec3 viewPos = camera.Position;
        particlesShader.setVec3("viewPos", viewPos);

        vec3 objectColor = vec3(0.0f, 0.5f, 0.8f);
        particlesShader.setVec3("objectColor", objectColor);

        glBindVertexArray(particlesInstanceVAO);
        glDrawElementsInstanced(GL_TRIANGLE_STRIP, sphereIndexCount, GL_UNSIGNED_INT, 0, PARTICLE_COUNT);
        glBindVertexArray(0);

        return 0;
    }

    int renderTerminateParticles() {
        glDeleteVertexArrays(1, &particlesInstanceVAO);
        glDeleteBuffers(1, &particlesInstanceVBO);
        glDeleteBuffers(1, &sphereVBO);
        glDeleteBuffers(1, &sphereEBO);

        sphereVertices.clear();
        sphereIndices.clear();

        return 0;
    }

    int renderInit() {
        windowInit();

        renderInitParticles();
        renderInitSkybox();

        return 0;
    }

    int render() {
        computeDeltaTime();
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderParticles();
        renderSkybox();

        glfwSwapBuffers(window);
        glfwPollEvents();

        return 0;
    }

    int renderTerminate() {
        renderTerminateParticles();
        renderTerminateSkybox();

        windowTerminate();

        return 0;
    }

    unsigned int loadTexture(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else {
            cout << "Texture failed to load at path: " << path << endl;
        }

        stbi_image_free(data);

        return textureID;
    }

    unsigned int loadCubemap(vector<string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        
        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            else {
                cout << "Cubemap texture failed to load at path: " << faces[i] << endl;
            }
            stbi_image_free(data);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    int generateSphere(const unsigned int SEGMENTS) {
        for (unsigned int x = 0; x < SEGMENTS; x++) {
            for (unsigned int y = 0; y < SEGMENTS; y++) {
                real xSegment = static_cast<real>(x) / static_cast<real>(SEGMENTS);
                real ySegment = static_cast<real>(y) / static_cast<real>(SEGMENTS);
                real xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
                real yPos = cos(ySegment * PI);
                real zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);

                // position
                sphereVertices.push_back(vec3(xPos, yPos, zPos));
                // normal
                sphereVertices.push_back(vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < SEGMENTS; y++) {
            if (!oddRow) {
                for (unsigned int x = 0; x <= SEGMENTS; x++) {
                    sphereIndices.push_back(y       * (SEGMENTS + 1) + x);
                    sphereIndices.push_back((y + 1) * (SEGMENTS + 1) + x);
                }
            }
            else {
                for (int x = SEGMENTS; x >= 0; x--) {
                    sphereIndices.push_back((y + 1) * (SEGMENTS + 1) + x);
                    sphereIndices.push_back(y       * (SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        sphereIndexCount = static_cast<unsigned int>(sphereIndices.size());

        return 0;
    }

}
