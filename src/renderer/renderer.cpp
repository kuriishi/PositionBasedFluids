#include "renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

#include <iostream>
#include <cmath>

#include "../common/common.hpp"
#include "shader.hpp"
#include "window.hpp"
#include "../simulator/simulator.hpp"

using glm::vec4;
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
using simulator::particlePositionSSBO;
using simulator::PARTICLE_COUNT;
using simulator::PARTICLE_RADIUS;

#define ENABLE_PROCESS_INPUT

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

    // particle 
    unsigned int particleVAO;
    unsigned int particleVBO;
    Shader particleShader;
    
    // FPS
    int deltaTimeCounter = 0;
    const int DELTA_TIME_COUNTER_MAX = 30;

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
    
    int renderInitParticle() {
        particleShader = Shader("src/renderer/shader/particle.vert", "src/renderer/shader/particle.frag");

        glGenVertexArrays(1, &particleVAO);
        glGenBuffers(1, &particleVBO);

        glBindVertexArray(particleVAO);

        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return 0;
    }

    int renderParticle() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        particleShader.use();

        mat4 view = camera.GetViewMatrix();
        particleShader.setMat4("view", view);

        mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        particleShader.setMat4("projection", projection);       

        // matrix.inverse.transpose.inverse = matrix.transpose
        mat4 viewMatrixTranspose = transpose(view);
        particleShader.setMat4("viewMatrixTranspose", viewMatrixTranspose);

        particleShader.setUint("HALF_PARTICLE_COUNT", PARTICLE_COUNT / 2);

        particleShader.setFloat("POINT_SIZE", static_cast<float>(PARTICLE_RADIUS));

        vec3 lightDir = vec3(1.0f, 1.0f, 0.5f);
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        float ambient = 0.1f;
        float diffuse = 1.0f;
        float specular = 0.5f;
        float shininess = 64.0f;
        particleShader.setVec3("light.direction", lightDir);
        particleShader.setVec3("light.color", lightColor);
        particleShader.setFloat("light.ambient", ambient);
        particleShader.setFloat("light.diffuse", diffuse);
        particleShader.setFloat("light.specular", specular);
        particleShader.setFloat("light.shininess", shininess);

        vec3 viewPos = camera.Position;
        particleShader.setVec3("viewPos", viewPos);

        glBindBuffer(GL_COPY_READ_BUFFER, particlePositionSSBO);
        glBindBuffer(GL_COPY_WRITE_BUFFER, particleVBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, PARTICLE_COUNT * sizeof(vec4));

        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        glBindVertexArray(0);

        return 0;
    }

    int renderTerminateParticle() {
        glDeleteVertexArrays(1, &particleVAO);

        glDeleteBuffers(1, &particleVBO);

        glDeleteProgram(particleShader.ID);

        return 0;
    }

    int renderInit() {
        renderInitParticle();
        renderInitSkybox();

        return 0;
    }

    int render() {
        computeDeltaTime();

        #ifdef ENABLE_PROCESS_INPUT
        processInput(window);
        #endif

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderParticle();
        renderSkybox();

        glfwSwapBuffers(window);
        glfwPollEvents();

        return 0;
    }

    int renderTerminate() {
        renderTerminateParticle();
        renderTerminateSkybox();

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

}
