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

namespace renderer {
    namespace fluid {
        // performance log
        const unsigned int QUERY_START_INDEX = common::SIMULATE_TIME_QUERY_COUNT + 2;
        
        // gui parameters
        DisplayMode displayMode = DisplayMode::FLUID;
        bool enableSmoothDepth = true;
        int smoothIteration = 8;
        float particleRadiusScaler = 2.0f;
        float minimumDensityScaler = 0.32f;
        float thicknessScaler = 0.05f;
        // rgb(66, 132, 244)
        float fluidColor[3] = {66.0f / 255.0f, 132.0f / 255.0f, 244.0f / 255.0f};

        // common shaders
        Shader renderFluidShader;
        Shader renderFluidDepthShader;
        Shader renderFluidNormalShader;
        Shader renderFluidThicknessShader;
        Shader particleShader;
        Shader renderFluidDepthTextureShader;
        Shader renderFluidThicknessTextureShader;
        ComputeShader clearCS;
        ComputeShader smoothFluidDepthHorizontalCS;
        ComputeShader smoothFluidDepthVerticalCS;
        ComputeShader computeFluidNormalCS;

        Fluid::Fluid(Camera& camera, unsigned int width, unsigned int height, std::shared_ptr<renderer::info::SceneInfo> sceneInfo) 
                    : m_info(std::make_shared<renderer::info::FluidInfo>(camera, width, height)),
                      m_sceneInfo(sceneInfo) {
            init();
        }

        int Fluid::init() {
            // init shaders
            {
            renderFluidShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluid.frag");
            renderFluidDepthShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidDepth.frag");
            renderFluidNormalShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidNormal.frag");
            renderFluidThicknessShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/fluidThickness.frag");
            particleShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/particle.frag");
            renderFluidDepthTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidDepthTexture.frag");
            renderFluidThicknessTextureShader = Shader("src/renderer/shader/fluid/fluid.vert", "src/renderer/shader/fluid/prepare/fluidThicknessTexture.frag");
            clearCS = ComputeShader("src/renderer/shader/fluid/prepare/clear.comp");
            smoothFluidDepthHorizontalCS = ComputeShader("src/renderer/shader/fluid/prepare/smoothFluidDepthHorizontal.comp");
            smoothFluidDepthVerticalCS = ComputeShader("src/renderer/shader/fluid/prepare/smoothFluidDepthVertical.comp");
            computeFluidNormalCS = ComputeShader("src/renderer/shader/fluid/prepare/computeFluidNormal.comp");
            }

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

            // aid depth texture for smoothing
            smoothedDepthAidTexture = utils::generateTextureR32F(m_info->width, m_info->height);

            // normal texture
            normalViewSpaceTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);

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

        int Fluid::smoothDepthTexture() {
            smoothFluidDepthHorizontalCS.use();
            smoothFluidDepthHorizontalCS.setInt("SCR_WIDTH", m_info->width);
            smoothFluidDepthHorizontalCS.setInt("SCR_HEIGHT", m_info->height);
            utils::bindTextureWithLayer0(m_info->smoothedDepthTexture, 0, GL_R32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(smoothedDepthAidTexture, 1, GL_R32F, GL_WRITE_ONLY);
            utils::bindTextureWithLayer0(m_info->validTexture, 2, GL_R8I, GL_READ_ONLY);
            smoothFluidDepthHorizontalCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            smoothFluidDepthVerticalCS.use();
            smoothFluidDepthVerticalCS.setInt("SCR_WIDTH", m_info->width);
            smoothFluidDepthVerticalCS.setInt("SCR_HEIGHT", m_info->height);
            utils::bindTextureWithLayer0(smoothedDepthAidTexture, 0, GL_R32F, GL_READ_ONLY);
            utils::bindTextureWithLayer0(m_info->smoothedDepthTexture, 1, GL_R32F, GL_WRITE_ONLY);
            utils::bindTextureWithLayer0(m_info->validTexture, 2, GL_R8I, GL_READ_ONLY);
            smoothFluidDepthVerticalCS.dispatchCompute(m_info->width * m_info->height);
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
                for (int i = 0; i < smoothIteration; i++) {
                    smoothDepthTexture();
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

        int Fluid::render(bool prepare) {
            m_info->setViewport();

            if (prepare) {
                renderPrepare();
            }

            switch (displayMode) {
                case DisplayMode::DEPTH:
                    renderDepth();
                    break;
                case DisplayMode::THICKNESS:
                    renderThickness();
                    break;
                case DisplayMode::NORMAL:
                    renderNormal();
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

            glDeleteProgram(renderFluidShader.ID);
            glDeleteProgram(renderFluidDepthShader.ID);
            glDeleteProgram(renderFluidDepthTextureShader.ID);
            glDeleteProgram(renderFluidThicknessTextureShader.ID);
            glDeleteProgram(smoothFluidDepthHorizontalCS.ID);
            glDeleteProgram(smoothFluidDepthVerticalCS.ID);
            glDeleteProgram(computeFluidNormalCS.ID);
        }       


        int Fluid::copyParticleAttribute() {
            utils::copySSBO2VBO(simulator::particlePositionSSBO, VBO, simulator::PARTICLE_COUNT * sizeof(glm::vec4));
            utils::copySSBO2VBO(simulator::densitySSBO, densityVBO, simulator::PARTICLE_COUNT * sizeof(float));

            return 0;
        }
    }
}
