#include "fluid.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "../common/shader.hpp"
#include "../common/compute_shader.hpp"
#include "../common/performance_log.hpp"
#include "../simulator/simulator.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "parameter.hpp"

namespace renderer {
    namespace fluid {
        // performance log
        const unsigned int QUERY_START_INDEX = common::SIMULATE_TIME_QUERY_COUNT + 2;
        
        // gui parameters
        DisplayMode displayMode = DisplayMode::CARTOON;
        bool enableSmoothDepth = true;
        int smoothIteration = 8;
        int smoothKernelRadius = 15;
        bool separateBilateralFilter = true;
        float particleRadiusScaler = 2.0f;
        float minimumDensityScaler = 0.2f;
        float thicknessScaler = 0.05f;
        // rgb(66, 132, 244)
        float fluidColor[3] = {66.0f / 255.0f, 132.0f / 255.0f, 244.0f / 255.0f};
        // cartoon
        float brightThreshold = 0.7f;
        float darkThreshold = 0.3f;
        float brightFactor = 1.5f;
        float darkFactor = 0.5f;
        float refractThreshold = 0.7f;
        float reflectThreshold = 0.4f;
        float refractMax = 0.7f;
        float reflectMax = 0.2f;
        float foamDensityScaler = 0.5f;
        int foamErodeKernelRadius = 4;
        int foamErodeMinimunNeighborCount = 64;
        int edgeKernelRadius = 3;

        // common shaders
        Shader renderFluidShader;
        Shader renderCartoonShader;
        Shader renderFluidDepthShader;
        Shader renderFluidNormalShader;
        Shader renderFluidThicknessShader;
        Shader particleShader;
        Shader renderFluidDepthTextureShader;
        Shader renderFluidThicknessTextureShader;
        Shader renderFoamTextureShader;
        Shader renderFoamShader;
        Shader renderEdgeShader;
        ComputeShader clearCS;
        ComputeShader smoothDepthCS;
        ComputeShader computeFluidNormalCS;
        ComputeShader erodeFoamTextureCS;
        ComputeShader edgeCS;
        ComputeShader fixInvalidNormalsCS;

        Fluid::Fluid(Camera& camera, unsigned int width, unsigned int height, std::shared_ptr<renderer::info::SceneInfo> sceneInfo) 
                    : m_info(std::make_shared<renderer::info::FluidInfo>(camera, width, height)),
                      m_sceneInfo(sceneInfo) {
            init();
        }

        int Fluid::init() {
            // init shaders
            {
            renderFluidShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluid.frag");
            renderCartoonShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/cartoon.frag");
            renderFluidDepthShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidDepth.frag");
            renderFluidNormalShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidNormal.frag");
            renderFluidThicknessShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidThickness.frag");
            particleShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/particle.frag");
            renderFluidDepthTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidDepthTexture.frag");
            renderFluidThicknessTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidThicknessTexture.frag");
            renderFoamTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/foamTexture.frag");
            renderFoamShader = Shader("src/renderer/shader/screenQuad.vert", "src/renderer/shader/fluid/foam.frag");
            renderEdgeShader = Shader("src/renderer/shader/screenQuad.vert", "src/renderer/shader/fluid/edge.frag");
            clearCS = ComputeShader("src/renderer/shader/fluid/prepare/clear.comp");
            smoothDepthCS = ComputeShader("src/renderer/shader/fluid/prepare/smoothDepth.comp");
            computeFluidNormalCS = ComputeShader("src/renderer/shader/fluid/prepare/computeFluidNormal.comp");
            erodeFoamTextureCS = ComputeShader("src/renderer/shader/fluid/prepare/erodeFoamTexture.comp");
            edgeCS = ComputeShader("src/renderer/shader/fluid/prepare/edge.comp");
            fixInvalidNormalsCS = ComputeShader("src/renderer/shader/fluid/prepare/fixInvalidNormals.comp");
            }
            ComputeShader erodeFoamTextureCS;
            erodeFoamTextureCS = ComputeShader("src/renderer/shader/fluid/prepare/erodeFoamTexture.comp");

            // whole framebuffer
            {
            glGenFramebuffers(1, &FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->colorTexture, 0);
            utils::bindColorAttachment(m_info->positionTexture, 1);
            utils::bindColorAttachment(m_info->normalTexture, 2);
            utils::setDrawbuffersLayout(2);
            }

            // depth framebuffer
            {
            glGenFramebuffers(1, &depthFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->smoothedDepthTexture, 0);
            utils::bindColorAttachment(m_info->validTexture, 1);
            utils::setDrawbuffersLayout(1);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {  
                std::cerr << "ERROR::FRAMEBUFFER:: Depth framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // thickness framebuffer
            {
            glGenFramebuffers(1, &thicknessFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, thicknessFBO);

            utils::bindDepthAttachment(m_info->depthTexture);

            thicknessTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);
            utils::bindColorAttachment(thicknessTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {  
                std::cerr << "ERROR::FRAMEBUFFER:: Thickness framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // foam framebuffer
            {
            glGenFramebuffers(1, &foamFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, foamFBO);

            utils::bindDepthAttachment(m_info->depthTexture);

            foamTexture = utils::generateTextureR8I(m_info->width, m_info->height);
            utils::bindColorAttachment(foamTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {  
                std::cerr << "ERROR::FRAMEBUFFER:: Foam framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            erodedFoamTexture = utils::generateTextureR8I(m_info->width, m_info->height);
            }

            // aid depth texture for smoothing
            smoothedDepthAidTexture = utils::generateTextureR32F(m_info->width, m_info->height);

            // normal texture
            normalViewSpaceTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);
            repairedNormalViewSpaceTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);

            edgeTexture = utils::generateTextureR8I(m_info->width, m_info->height);

            // vertex array and buffer
            {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &densityVBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, simulator::PARTICLE_COUNT * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, densityVBO);
            glBufferData(GL_ARRAY_BUFFER, simulator::PARTICLE_COUNT * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            }

            return 0;
        }

        int Fluid::clear() {
            clearCS.use();
            clearCS.setInt("SCR_WIDTH", m_info->width);
            clearCS.setInt("SCR_HEIGHT", m_info->height);
            utils::bindTextureWithLayer0(m_info->validTexture, 0, GL_R8I, GL_WRITE_ONLY);
            clearCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return 0;
        }

        int Fluid::renderDepthTexture() {
            glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            renderFluidDepthTextureShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidDepthTextureShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidDepthTextureShader.setMat4("uProjection", uProjection);       
            // other parameters
            renderFluidDepthTextureShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidDepthTextureShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::renderThicknessTexture() {
            glBindFramebuffer(GL_FRAMEBUFFER, thicknessFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            renderFluidThicknessTextureShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidThicknessTextureShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidThicknessTextureShader.setMat4("uProjection", uProjection);       
            // other parameters
            renderFluidThicknessTextureShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidThicknessTextureShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            renderFluidThicknessTextureShader.setFloat("uThicknessScaler", thicknessScaler);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::smoothDepthTexture(int kernelRadius) {
            smoothDepthCS.use();
            smoothDepthCS.setInt("SCR_WIDTH", m_info->width);
            smoothDepthCS.setInt("SCR_HEIGHT", m_info->height);
            smoothDepthCS.setInt("uKernelRadius", kernelRadius);
            smoothDepthCS.setInt("uSeparate", separateBilateralFilter ? 1 : 0);
            utils::bindTextureWithLayer0(m_info->validTexture, 2, GL_R8I, GL_READ_ONLY);

            // horizontal
            smoothDepthCS.setInt("uHorizontal", 1);
            utils::bindTextureWithLayer0(m_info->smoothedDepthTexture, 0, GL_R32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(smoothedDepthAidTexture, 1, GL_R32F, GL_WRITE_ONLY);
            smoothDepthCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // vertical
            smoothDepthCS.setInt("uHorizontal", 0);
            utils::bindTextureWithLayer0(smoothedDepthAidTexture, 0, GL_R32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(m_info->smoothedDepthTexture, 1, GL_R32F, GL_WRITE_ONLY);
            smoothDepthCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return 0;
        }

        int Fluid::computeNormalTexture() {
            computeFluidNormalCS.use();
            computeFluidNormalCS.setInt("SCR_WIDTH", m_info->width);
            computeFluidNormalCS.setInt("SCR_HEIGHT", m_info->height);
            glm::mat4 projection = m_info->projection();
            glm::mat4 projectionInverse = glm::inverse(projection);
            computeFluidNormalCS.setMat4("projectionInverse", projectionInverse);

            utils::bindTextureWithLayer0(m_info->smoothedDepthTexture, 0, GL_R32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(normalViewSpaceTexture, 1, GL_RGBA32F, GL_WRITE_ONLY);

            computeFluidNormalCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            fixInvalidNormals();

            return 0;
        }

        int Fluid::fixInvalidNormals() {
            fixInvalidNormalsCS.use();
            fixInvalidNormalsCS.setIvec2("uResolution", glm::ivec2(m_info->width, m_info->height));
            utils::bindTextureWithLayer0(normalViewSpaceTexture, 0, GL_RGBA32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(repairedNormalViewSpaceTexture, 1, GL_RGBA32F, GL_WRITE_ONLY);

            fixInvalidNormalsCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            utils::copyTexture2D(repairedNormalViewSpaceTexture, normalViewSpaceTexture, m_info->width, m_info->height);

            return 0;
        }

        int Fluid::renderPrepare() {
            copyParticleAttribute();

            {
            common::queryTime(QUERY_START_INDEX + 1);
            }

            clear();

            renderDepthTexture();

            {
            common::queryTime(QUERY_START_INDEX + 2);
            }

            if (enableSmoothDepth) {
                float kernelRadius = static_cast<float>(smoothKernelRadius);
                float delta = static_cast<float>(smoothKernelRadius - 1) / (smoothIteration - 1);
                for (int i = 0; i < smoothIteration; i++) {
                    smoothDepthTexture(static_cast<int>(kernelRadius));
                    kernelRadius -= delta;
                }
            }

            {
            common::queryTime(QUERY_START_INDEX + 3);
            }

            computeNormalTexture();

            {
            common::queryTime(QUERY_START_INDEX + 4);
            }

            renderThicknessTexture();

            return 0;
        }


        int Fluid::renderFluid() {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);

            renderFluidShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidShader.setMat4("uProjection", uProjection);
            glm::mat4 viewInverse = glm::inverse(uView);
            renderFluidShader.setMat4("uViewInverse", viewInverse);
            glm::mat4 viewTranspose = glm::transpose(uView);
            renderFluidShader.setMat4("uViewTranspose", viewTranspose);
            // parameters
            renderFluidShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            renderFluidShader.setVec3("uFluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));
            renderFluidShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            renderFluidShader.setVec3("uCameraPosition", m_info->position());
            // textures
            utils::bindTexture2D(renderFluidShader, "uNormalViewSpaceTexture", normalViewSpaceTexture, 0);
            utils::bindTexture2D(renderFluidShader, "uThicknessTexture", thicknessTexture, 1);
            utils::bindTexture2D(renderFluidShader, "uSceneColorTexture", m_sceneInfo->colorTexture, 2);
            utils::bindTexture2D(renderFluidShader, "uSmoothedDepthTexture", m_info->smoothedDepthTexture, 3);
            utils::bindTextureCubeMap(renderFluidShader, "uSkyboxTexture", m_sceneInfo->skyboxTexture, 4);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);
            glBindVertexArray(0);

            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }



        int Fluid::renderDepth() {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            renderFluidDepthShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidDepthShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidDepthShader.setMat4("uProjection", uProjection);
            glm::mat4 viewInverse = glm::inverse(uView);
            renderFluidDepthShader.setMat4("uViewInverse", viewInverse);
            glm::mat4 viewTranspose = glm::transpose(uView);
            renderFluidDepthShader.setMat4("uViewTranspose", viewTranspose);
            // parameters
            renderFluidDepthShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            renderFluidDepthShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidDepthShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            // textures
            utils::bindTexture2D(renderFluidDepthShader, "uNormalViewSpaceTexture", normalViewSpaceTexture, 0);
            utils::bindTexture2D(renderFluidDepthShader, "uSmoothedDepthTexture", m_info->smoothedDepthTexture, 1);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

            glBindVertexArray(0);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::renderThickness() {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO); 
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            renderFluidThicknessShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidThicknessShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidThicknessShader.setMat4("uProjection", uProjection);
            glm::mat4 viewInverse = glm::inverse(uView);
            renderFluidThicknessShader.setMat4("uViewInverse", viewInverse);
            glm::mat4 viewTranspose = glm::transpose(uView);
            renderFluidThicknessShader.setMat4("uViewTranspose", viewTranspose);
            // parameters
            renderFluidThicknessShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            renderFluidThicknessShader.setVec3("uFluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));
            renderFluidThicknessShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidThicknessShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            // textures
            utils::bindTexture2D(renderFluidThicknessShader, "uNormalViewSpaceTexture", normalViewSpaceTexture, 0);
            utils::bindTexture2D(renderFluidThicknessShader, "uThicknessTexture", thicknessTexture, 1);
            utils::bindTexture2D(renderFluidThicknessShader, "uSmoothedDepthTexture", m_info->smoothedDepthTexture, 2);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

            glBindVertexArray(0);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::renderNormal() {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            renderFluidNormalShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderFluidNormalShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFluidNormalShader.setMat4("uProjection", uProjection);
            glm::mat4 uViewInverse = glm::inverse(uView);
            renderFluidNormalShader.setMat4("uViewInverse", uViewInverse);
            glm::mat4 uViewTranspose = glm::transpose(uView);
            renderFluidNormalShader.setMat4("uViewTranspose", uViewTranspose);
            // other parameters
            renderFluidNormalShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            renderFluidNormalShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderFluidNormalShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            utils::bindTexture2D(renderFluidNormalShader, "uNormalViewSpaceTexture", normalViewSpaceTexture, 0);
            utils::bindTexture2D(renderFluidNormalShader, "uSmoothedDepthTexture", m_info->smoothedDepthTexture, 1);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);

            glBindVertexArray(0);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);


            return 0;
        }

        int Fluid::renderParticle() {
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            particleShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            particleShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            particleShader.setMat4("uProjection", uProjection);
            glm::mat4 uViewInverse = glm::inverse(uView);
            particleShader.setMat4("uViewInverse", uViewInverse);
            glm::mat4 uViewTranspose = glm::transpose(uView);
            particleShader.setMat4("uViewTranspose", uViewTranspose);
            // other parameters
            particleShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            particleShader.setVec3("uFluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));
            particleShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            particleShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            // light
            glm::vec3 lightDir = glm::vec3(1.0f, 1.0f, 0.5f);
            glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
            float ambient = 0.1f;
            float diffuse = 1.0f;
            float specular = 0.5f;
            float shininess = 64.0f;
            particleShader.setVec3("uLight.direction", lightDir);
            particleShader.setVec3("uLight.color", lightColor);
            particleShader.setFloat("uLight.ambient", ambient);
            particleShader.setFloat("uLight.diffuse", diffuse);
            particleShader.setFloat("uLight.specular", specular);
            particleShader.setFloat("uLight.shininess", shininess);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);
            glBindVertexArray(0);

            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::renderFoamTexture() {
            glBindFramebuffer(GL_FRAMEBUFFER, foamFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            renderFoamTextureShader.use();
            glm::mat4 uView = m_info->view();
            renderFoamTextureShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderFoamTextureShader.setMat4("uProjection", uProjection);
            renderFoamTextureShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            // other parameters
            renderFoamTextureShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            renderFoamTextureShader.setFloat("uFoamDensity", static_cast<float>(foamDensityScaler * simulator::REST_DENSITY));

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);
            glBindVertexArray(0);

            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::erodeFoamTexture() {
            erodeFoamTextureCS.use();
            erodeFoamTextureCS.setIvec2("uResolution", glm::ivec2(m_info->width, m_info->height));
            erodeFoamTextureCS.setInt("uKernelRadius", foamErodeKernelRadius);
            erodeFoamTextureCS.setInt("uMinimunNeighborCount", foamErodeMinimunNeighborCount);
            utils::bindTextureWithLayer0(foamTexture, 3, GL_R8I, GL_READ_ONLY);
            utils::bindTextureWithLayer0(erodedFoamTexture, 4, GL_R8I, GL_WRITE_ONLY);

            erodeFoamTextureCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return 0;
        }

        int Fluid::renderFoam() {
            renderFoamTexture();
            erodeFoamTexture();

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            
            if (displayMode == DisplayMode::FOAM) {
                glClear(GL_COLOR_BUFFER_BIT);
            }
            
            renderFoamShader.use();
            utils::bindTexture2D(renderFoamShader, "uFoamTexture", erodedFoamTexture, 0);

            utils::drawScreenQuad();

            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::computeEdgeTexture() {
            edgeCS.use();
            edgeCS.setInt("uKernelRadius", edgeKernelRadius);
            edgeCS.setIvec2("uResolution", glm::ivec2(m_info->width, m_info->height));
            utils::bindTextureWithLayer0(m_info->validTexture, 0, GL_R8I, GL_READ_ONLY);
            utils::bindTextureWithLayer0(edgeTexture, 1, GL_R8I, GL_WRITE_ONLY);

            edgeCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            utils::copyTexture2D(edgeTexture, m_info->validTexture, m_info->width, m_info->height);

            return 0;
        }

        int Fluid::renderEdge() {
            computeEdgeTexture();

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            renderEdgeShader.use();
            utils::bindTexture2D(renderEdgeShader, "uEdgeTexture", edgeTexture, 0);

            utils::drawScreenQuad();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Fluid::renderCartoon() {
            // renderEdge();

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // glClear(GL_DEPTH_BUFFER_BIT);

            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);

            renderCartoonShader.use();
            // MVP transform
            glm::mat4 uView = m_info->view();
            renderCartoonShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            renderCartoonShader.setMat4("uProjection", uProjection);
            glm::mat4 viewInverse = glm::inverse(uView);
            renderCartoonShader.setMat4("uViewInverse", viewInverse);
            glm::mat4 viewTranspose = glm::transpose(uView);
            renderCartoonShader.setMat4("uViewTranspose", viewTranspose);
            // parameters
            renderCartoonShader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            renderCartoonShader.setVec3("uFluidColor", glm::vec3(fluidColor[0], fluidColor[1], fluidColor[2]));
            renderCartoonShader.setFloat("uPointSize", static_cast<float>(simulator::PARTICLE_RADIUS * particleRadiusScaler));
            renderCartoonShader.setFloat("uMinimumDensity", static_cast<float>(minimumDensityScaler * simulator::REST_DENSITY));
            renderCartoonShader.setVec3("uCameraPosition", m_info->position());
            // textures
            utils::bindTexture2D(renderCartoonShader, "uNormalViewSpaceTexture", normalViewSpaceTexture, 0);
            utils::bindTexture2D(renderCartoonShader, "uThicknessTexture", thicknessTexture, 1);
            utils::bindTexture2D(renderCartoonShader, "uSceneColorTexture", m_sceneInfo->colorTexture, 2);
            utils::bindTexture2D(renderCartoonShader, "uSmoothedDepthTexture", m_info->smoothedDepthTexture, 3);
            utils::bindTextureCubeMap(renderCartoonShader, "uSkyboxTexture", m_sceneInfo->skyboxTexture, 4);
            utils::bindTexture2D(renderCartoonShader, "uValidTexture", m_info->validTexture, 5);
            // cartoon
            renderCartoonShader.setFloat("uBrightThreshold", brightThreshold);
            renderCartoonShader.setFloat("uBrightFactor", brightFactor);
            renderCartoonShader.setFloat("uDarkThreshold", darkThreshold);
            renderCartoonShader.setFloat("uDarkFactor", darkFactor);
            renderCartoonShader.setFloat("uRefractThreshold", refractThreshold);
            renderCartoonShader.setFloat("uRefractMax", refractMax);
            renderCartoonShader.setFloat("uReflectThreshold", reflectThreshold);
            renderCartoonShader.setFloat("uReflectMax", reflectMax);
            renderCartoonShader.setInt("uEdgeKernelRadius", edgeKernelRadius);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, simulator::PARTICLE_COUNT);
            glBindVertexArray(0);

            glDisable(GL_PROGRAM_POINT_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            renderFoam();

            return 0;
        }
        
        int Fluid::render(bool prepare) {
            m_info->setViewport();

            if (prepare) {
                renderPrepare();
            }

            switch (displayMode) {
                case DisplayMode::FLUID:
                    renderFluid();
                    break;
                case DisplayMode::CARTOON:
                    renderCartoon();
                    break;
                case DisplayMode::FOAM:
                    renderFoam();
                    break;
                case DisplayMode::DEPTH:
                    renderDepth();
                    break;
                case DisplayMode::THICKNESS:
                    renderThickness();
                    break;
                case DisplayMode::NORMAL:
                    renderNormal();
                    break;
                case DisplayMode::PARTICLE:
                    renderParticle();
                    break;
                default:
                    renderFluid();
            }

            return 0;
        }

        int Fluid::setSceneInfo(std::shared_ptr<renderer::info::SceneInfo> sceneInfo) {
            m_sceneInfo = sceneInfo;
            return 0;
        }

        Fluid::~Fluid() {
            glDeleteFramebuffers(1, &depthFBO);
            glDeleteFramebuffers(1, &thicknessFBO);
            glDeleteTextures(1, &smoothedDepthAidTexture);
            glDeleteTextures(1, &normalViewSpaceTexture);
            glDeleteTextures(1, &repairedNormalViewSpaceTexture);

            glDeleteProgram(renderFluidShader.ID);
            glDeleteProgram(renderCartoonShader.ID);
            glDeleteProgram(renderFluidDepthShader.ID);
            glDeleteProgram(renderFluidDepthTextureShader.ID);
            glDeleteProgram(renderFluidThicknessTextureShader.ID);
            glDeleteProgram(smoothDepthCS.ID);
            glDeleteProgram(computeFluidNormalCS.ID);
            glDeleteProgram(fixInvalidNormalsCS.ID);
        }       


        int Fluid::copyParticleAttribute() {
            utils::copySSBO2VBO(simulator::particlePositionSSBO, VBO, simulator::PARTICLE_COUNT * sizeof(glm::vec4));
            utils::copySSBO2VBO(simulator::densitySSBO, densityVBO, simulator::PARTICLE_COUNT * sizeof(float));

            return 0;
        }
    }
}
