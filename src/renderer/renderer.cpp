#include "renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

#include <iostream>
#include <cmath>
#include <iomanip>

#include "../common/common.hpp"
#include "../common/shader.hpp"
#include "../common/compute_shader.hpp"
#include "../common/performance_log.hpp"
#include "window.hpp"
#include "../simulator/simulator.hpp"

namespace renderer {
    // gui parameters
    DisplayMode displayMode = DisplayMode::FLUID;
    bool enableSmoothDepth = true;
    int smoothIteration = 8;
    float particleRadiusScaler = 2.0f;
    float minimumDensityScaler = 0.32f;
    float thicknessScaler = 0.05f;
    // rgb(66, 132, 244)
    float fluidColor[3] = {66.0f / 255.0f, 132.0f / 255.0f, 244.0f / 255.0f};

    // performance log
    const unsigned int QUERY_START_INDEX = common::SIMULATE_TIME_QUERY_COUNT + 2;

    // screen quad
    std::vector<float> screenQuadVertices = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int screenQuadVAO;
    unsigned int screenQuadVBO;

    // background
    unsigned int backgroundDepthTexture;
    unsigned int backgroundTexture;
    unsigned int backgroundFBO;
    Shader backgroundShader;

    // skybox
    std::vector<float> skyboxVertices = {
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
    std::vector<std::string> skyboxFaces = {
        "resource/skybox/right.jpg",
        "resource/skybox/left.jpg",
        "resource/skybox/top.jpg",
        "resource/skybox/bottom.jpg",
        "resource/skybox/front.jpg",
        "resource/skybox/back.jpg"
    };
    unsigned int skyboxVAO;
    unsigned int skyboxVBO;
    unsigned int skyboxTexture;
    Shader skyboxShader;

    // floor
    std::vector<float> floorVertices = {
        // positions          // normals         // texCoords
        -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

         1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    };
    const float FLOOR_SIZE = 2.0;
    unsigned int floorVAO;
    unsigned int floorVBO;
    unsigned int floorTexture;
    Shader floorShader;

    // fluid
    unsigned int fluidDepthFBO;
    unsigned int fluidThicknessFBO;
    unsigned int fluidVAO;
    unsigned int fluidVBO;
    unsigned int fluidDensityVBO;
    unsigned int fluidDepthTexture;
    unsigned int fluidSmoothedDepthTexture;
    unsigned int fluidSmoothedDepthAidTexture;
    unsigned int fluidSampledFlagTexture;
    unsigned int fluidThicknessTexture;
    unsigned int fluidNormalViewSpaceTexture;
    unsigned int fluidTexture;
    Shader renderFluidShader;
    Shader renderFluidDepthShader;
    Shader renderFluidNormalShader;
    Shader renderFluidThicknessShader;
    Shader particleShader;
    Shader renderFluidDepthTextureShader;
    Shader renderFluidThicknessTextureShader;
    ComputeShader clearFlagCS;
    ComputeShader smoothFluidDepthHorizontalCS;
    ComputeShader smoothFluidDepthVerticalCS;
    ComputeShader computeFluidNormalCS;

    // background
    int renderInitSkybox() {
        skyboxShader = Shader("src/renderer/shader/background/skybox/skybox.vert", "src/renderer/shader/background/skybox/skybox.frag");

        stbi_set_flip_vertically_on_load(false);
        skyboxTexture = loadCubemap(skyboxFaces);
        skyboxShader.use();
        skyboxShader.setInt("skyboxTexture", 0);

        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glBindVertexArray(0);

        return 0;
    }

    int renderSkybox() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        skyboxShader.use();

        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
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
        skyboxFaces.clear();

        return 0;
    }
    
    int renderInitFloor() {
        floorShader = Shader("src/renderer/shader/background/floor/floor.vert", "src/renderer/shader/background/floor/floor.frag");

        stbi_set_flip_vertically_on_load(false);
        floorTexture = loadTexture("resource/floor.jpg");

        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);

        glBindVertexArray(floorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(float), &floorVertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return 0;
    }

    int renderFloor() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        floorShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(FLOOR_SIZE, 1.0f, FLOOR_SIZE));
        floorShader.setMat4("model", model);
        glm::mat4 view = camera.GetViewMatrix();
        floorShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        floorShader.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        floorShader.setInt("floorTexture", 0);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        return 0;
    }

    int renderTerminateFloor() {
        glDeleteVertexArrays(1, &floorVAO);
        glDeleteBuffers(1, &floorVBO);
        glDeleteTextures(1, &floorTexture);
        glDeleteProgram(floorShader.ID);

        floorVertices.clear();

        return 0;
    }

    int renderInitBackground() {
        backgroundShader = Shader("src/renderer/shader/screenQuad.vert", "src/renderer/shader/background/background.frag");

        // framebuffer
        {
        glGenFramebuffers(1, &backgroundFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);

        // depth texture
        {
        glGenTextures(1, &backgroundDepthTexture);
        glBindTexture(GL_TEXTURE_2D, backgroundDepthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, backgroundDepthTexture, 0);
        }

        // background texture
        {
        glGenTextures(1, &backgroundTexture);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Background framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // vertex array and buffer
        {
        glGenVertexArrays(1, &screenQuadVAO);
        glGenBuffers(1, &screenQuadVBO);

        glBindVertexArray(screenQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, screenQuadVertices.size() * sizeof(float), &screenQuadVertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        }

        renderInitSkybox();
        renderInitFloor();

        return 0;
    }

    int renderBackground() {
        renderSkybox();
        renderFloor();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return 0;
    }

    int drawBackground() {
        backgroundShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        backgroundShader.setInt("backgroundTexture", 0);

        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        return 0;
    }

    int renderTerminateBackground() {
        renderTerminateSkybox();

        glDeleteFramebuffers(1, &backgroundFBO);
        glDeleteVertexArrays(1, &screenQuadVAO);
        glDeleteBuffers(1, &screenQuadVBO);
        glDeleteTextures(1, &backgroundTexture);
        glDeleteTextures(1, &backgroundDepthTexture);

        screenQuadVertices.clear();

        return 0;
    }

    // fluid
    int renderInitFluid() {
        // init shaders
        {
        renderFluidShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluid.frag");
        renderFluidDepthShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidDepth.frag");
        renderFluidNormalShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidNormal.frag");
        renderFluidThicknessShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidThickness.frag");
        particleShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/particle.frag");
        renderFluidDepthTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidDepthTexture.frag");
        renderFluidThicknessTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidThicknessTexture.frag");
        clearFlagCS = ComputeShader("src/renderer/shader/fluid/prepare/clearFlag.comp");
        smoothFluidDepthHorizontalCS = ComputeShader("src/renderer/shader/fluid/prepare/smoothFluidDepthHorizontal.comp");
        smoothFluidDepthVerticalCS = ComputeShader("src/renderer/shader/fluid/prepare/smoothFluidDepthVertical.comp");
        computeFluidNormalCS = ComputeShader("src/renderer/shader/fluid/prepare/computeFluidNormal.comp");
        }

        // fluid depth framebuffer
        {
        glGenFramebuffers(1, &fluidDepthFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, fluidDepthFBO);

        // depth texture
        glGenTextures(1, &fluidDepthTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidDepthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fluidDepthTexture, 0);

        // smoothed depth texture
        glGenTextures(1, &fluidSmoothedDepthTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fluidSmoothedDepthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluidSmoothedDepthTexture, 0);

        // sampled flag texture
        glGenTextures(1, &fluidSampledFlagTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fluidSampledFlagTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8I, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fluidSampledFlagTexture, 0);

        // layout
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {  
            std::cerr << "ERROR::FRAMEBUFFER:: Depth framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // fluid thickness framebuffer
        {
        glGenFramebuffers(1, &fluidThicknessFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, fluidThicknessFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fluidDepthTexture, 0);

        // thickness texture
        glGenTextures(1, &fluidThicknessTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fluidThicknessTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluidThicknessTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {  
            std::cerr << "ERROR::FRAMEBUFFER:: Thickness framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // aid depth texture for smoothing
        {
        glGenTextures(1, &fluidSmoothedDepthAidTexture);
        glBindTexture(GL_TEXTURE_2D, fluidSmoothedDepthAidTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SCR_WIDTH, SCR_HEIGHT);
        }

        // normal texture
        {
        glGenTextures(1, &fluidNormalViewSpaceTexture);
        glBindTexture(GL_TEXTURE_2D, fluidNormalViewSpaceTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
        }

        // vertex array and buffer
        {
        glGenVertexArrays(1, &fluidVAO);
        glGenBuffers(1, &fluidVBO);
        glGenBuffers(1, &fluidDensityVBO);

        glBindVertexArray(fluidVAO);

        glBindBuffer(GL_ARRAY_BUFFER, fluidVBO);
        glBufferData(GL_ARRAY_BUFFER, simulator::PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, fluidDensityVBO);
        glBufferData(GL_ARRAY_BUFFER, simulator::PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        }

        return 0;
    }

    int clearFlag() {
        clearFlagCS.use();
        clearFlagCS.setInt("SCR_WIDTH", SCR_WIDTH);
        clearFlagCS.setInt("SCR_HEIGHT", SCR_HEIGHT);
        glBindImageTexture(0, fluidSampledFlagTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8I);
        glDispatchCompute(static_cast<unsigned int>(ceil(SCR_WIDTH * SCR_HEIGHT / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return 0;
    }

    int renderFluidDepthTexture() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        renderFluidDepthTextureShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidDepthTextureShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidDepthTextureShader.setMat4("projection", projection);       

        renderFluidDepthTextureShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        renderFluidDepthTextureShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderFluidThicknessTexture() {
        glBindFramebuffer(GL_FRAMEBUFFER, fluidThicknessFBO);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        renderFluidThicknessTextureShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidThicknessTextureShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidThicknessTextureShader.setMat4("projection", projection);       

        renderFluidThicknessTextureShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        renderFluidThicknessTextureShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
        renderFluidThicknessTextureShader.setFloat("THICKNESS_SCALER", thicknessScaler);

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int smoothFluidDepthTexture() {
        smoothFluidDepthHorizontalCS.use();
        smoothFluidDepthHorizontalCS.setInt("SCR_WIDTH", SCR_WIDTH);
        smoothFluidDepthHorizontalCS.setInt("SCR_HEIGHT", SCR_HEIGHT);
        glBindImageTexture(0, fluidSmoothedDepthTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, fluidSmoothedDepthAidTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glBindImageTexture(2, fluidSampledFlagTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8I);
        glDispatchCompute(static_cast<unsigned int>(ceil(SCR_WIDTH * SCR_HEIGHT / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        smoothFluidDepthVerticalCS.use();
        smoothFluidDepthVerticalCS.setInt("SCR_WIDTH", SCR_WIDTH);
        smoothFluidDepthVerticalCS.setInt("SCR_HEIGHT", SCR_HEIGHT);
        glBindImageTexture(0, fluidSmoothedDepthAidTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, fluidSmoothedDepthTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glBindImageTexture(2, fluidSampledFlagTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8I);
        glDispatchCompute(static_cast<unsigned int>(ceil(SCR_WIDTH * SCR_HEIGHT / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return 0;
    }

    int computeFluidNormalTexture() {
        computeFluidNormalCS.use();
        computeFluidNormalCS.setInt("SCR_WIDTH", SCR_WIDTH);
        computeFluidNormalCS.setInt("SCR_HEIGHT", SCR_HEIGHT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projectionInverse = glm::inverse(projection);
        computeFluidNormalCS.setMat4("projectionInverse", projectionInverse);

        glBindImageTexture(0, fluidSmoothedDepthTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, fluidNormalViewSpaceTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glDispatchCompute(static_cast<unsigned int>(ceil(SCR_WIDTH * SCR_HEIGHT / common::INVOCATION_PER_WORKGROUP)), 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return 0;
    }

    int renderFluidPrepare() {
        copyParticleAttribute();

        {
        common::queryTime(QUERY_START_INDEX + 1);
        }

        clearFlag();

        glBindFramebuffer(GL_FRAMEBUFFER, fluidDepthFBO);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        renderFluidDepthTexture();

        {
        common::queryTime(QUERY_START_INDEX + 2);
        }

        if (enableSmoothDepth) {
            for (int i = 0; i < smoothIteration; i++) {
                smoothFluidDepthTexture();
            }
        }

        {
        common::queryTime(QUERY_START_INDEX + 3);
        }

        computeFluidNormalTexture();

        {
        common::queryTime(QUERY_START_INDEX + 4);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fluidThicknessFBO);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        renderFluidThicknessTexture();

        return 0;
    }


    int renderFluid() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        renderFluidShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidShader.setMat4("projection", projection);       
        glm::mat4 viewMatrixInverse = glm::inverse(view);
        renderFluidShader.setMat4("viewMatrixInverse", viewMatrixInverse);
        glm::mat4 viewMatrixTranspose = glm::transpose(view);
        renderFluidShader.setMat4("viewMatrixTranspose", viewMatrixTranspose);

        renderFluidShader.setVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        renderFluidShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
        renderFluidShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));

        renderFluidShader.setInt("fluidNormalViewSpaceTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidNormalViewSpaceTexture);
        renderFluidShader.setInt("fluidThicknessTexture", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fluidThicknessTexture);
        renderFluidShader.setInt("backgroundTexture", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        renderFluidShader.setInt("skyboxTexture", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        renderFluidShader.setVec3("fluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);

        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderFluidDepth() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        renderFluidDepthShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidDepthShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidDepthShader.setMat4("projection", projection);       

        renderFluidDepthShader.setVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        renderFluidDepthShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        renderFluidDepthShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));

        renderFluidDepthShader.setInt("fluidSmoothedDepthTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidSmoothedDepthTexture);

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderFluidThickness() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        renderFluidThicknessShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidThicknessShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidThicknessShader.setMat4("projection", projection);
        
        renderFluidThicknessShader.setVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        renderFluidThicknessShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        renderFluidThicknessShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
        renderFluidThicknessShader.setVec3("fluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));

        renderFluidThicknessShader.setInt("fluidThicknessTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidThicknessTexture);

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderFluidNormal() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        renderFluidNormalShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        renderFluidNormalShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderFluidNormalShader.setMat4("projection", projection);
        glm::mat4 viewMatrixTranspose = glm::transpose(view);
        renderFluidNormalShader.setMat4("viewMatrixTranspose", viewMatrixTranspose);

        renderFluidNormalShader.setVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        renderFluidNormalShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        renderFluidNormalShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));

        renderFluidNormalShader.setInt("fluidNormalViewSpaceTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fluidNormalViewSpaceTexture);

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

        glBindVertexArray(0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderParticle() {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_DEPTH_TEST);

        particleShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        particleShader.setMat4("view", view);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        particleShader.setMat4("projection", projection);       

        glm::mat4 viewMatrixTranspose = glm::transpose(view);
        particleShader.setMat4("viewMatrixTranspose", viewMatrixTranspose);

        particleShader.setUint("HALF_PARTICLE_COUNT", simulator::PARTICLE_COUNT / 2);

        particleShader.setFloat("POINT_SIZE", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
        particleShader.setVec3("fluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));
        particleShader.setFloat("MINIMUN_DENSITY", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));


        glm::vec3 lightDir = glm::vec3(1.0f, 1.0f, 0.5f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
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

        glm::vec3 viewPos = camera.Position;
        particleShader.setVec3("viewPos", viewPos);

        glBindVertexArray(fluidVAO);
        glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);
        glBindVertexArray(0);

        glDisable(GL_PROGRAM_POINT_SIZE);

        return 0;
    }

    int renderTerminateFluid() {
        glDeleteFramebuffers(1, &fluidDepthFBO);
        glDeleteFramebuffers(1, &fluidThicknessFBO);
        glDeleteTextures(1, &fluidDepthTexture);
        glDeleteTextures(1, &fluidSmoothedDepthTexture);
        glDeleteTextures(1, &fluidSmoothedDepthAidTexture);
        glDeleteTextures(1, &fluidNormalViewSpaceTexture);

        glDeleteProgram(renderFluidShader.ID);
        glDeleteProgram(renderFluidDepthShader.ID);
        glDeleteProgram(renderFluidDepthTextureShader.ID);
        glDeleteProgram(renderFluidThicknessTextureShader.ID);
        glDeleteProgram(smoothFluidDepthHorizontalCS.ID);
        glDeleteProgram(smoothFluidDepthVerticalCS.ID);
        glDeleteProgram(computeFluidNormalCS.ID);

        return 0;
    }

    // renderer
    int renderInit() {
        glClearColor(0.2f, 0.4f, 0.8f, 1.0f);

        renderInitBackground();
        renderInitFluid();

        glFinish();

        return 0;
    }

    int render() {
        glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderBackground();

        {
        common::queryTime(QUERY_START_INDEX + 0);
        }

        renderFluidPrepare();

        {
        common::queryTime(QUERY_START_INDEX + 5);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderBackground();
        switch (displayMode) {
            case DisplayMode::DEPTH:
                renderFluidDepth();
                break;
            case DisplayMode::THICKNESS:
                renderFluidThickness();
                break;
            case DisplayMode::NORMAL:
                renderFluidNormal();
                break;
            case DisplayMode::FLUID:
                renderFluid();
                break;
            case DisplayMode::PARTICLE:
                renderParticle();
                break;
            default:
                renderFluid();
        }

        {
        common::queryTime(QUERY_START_INDEX + 6);
        }

        return 0;
    }

    int renderTerminate() {
        renderTerminateBackground();
        renderTerminateFluid();

        glFinish();

        return 0;
    }

    // utils
    int copyParticleAttribute() {
        glBindBuffer(GL_COPY_READ_BUFFER, simulator::particlePositionSSBO);
        glBindBuffer(GL_COPY_WRITE_BUFFER, fluidVBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, simulator::PARTICLE_COUNT * sizeof(glm::vec4));

        glBindBuffer(GL_COPY_READ_BUFFER, simulator::densitySSBO);
        glBindBuffer(GL_COPY_WRITE_BUFFER, fluidDensityVBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, simulator::PARTICLE_COUNT * sizeof(float));

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
            std::cerr << "Texture failed to load at path: " << path << std::endl;
        }

        stbi_image_free(data);

        return textureID;
    }

    unsigned int loadCubemap(std::vector<std::string> faces) {
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
                std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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
