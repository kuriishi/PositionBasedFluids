#include "caustics.hpp"

#include <iostream>

#include "../common/shader.hpp"
#include "../common/compute_shader.hpp"
#include "window.hpp"
#include "scene.hpp"
#include "fluid.hpp"
#include "utils.hpp"

namespace renderer {
    namespace caustics {
        ComputeShader write2photonVBOCS;
        Shader shader;

        Caustics::Caustics(Camera& camera, unsigned int width, unsigned int height) {
            this->m_info = std::make_shared<renderer::info::CausticsInfo>(camera, width, height);
            this->m_scene = std::make_shared<renderer::scene::Scene>(camera, width, height);
            this->m_fluid = std::make_shared<renderer::fluid::Fluid>(camera, width, height, m_scene->m_info);

            init();
        }

        Caustics::~Caustics() {
            terminate();
        }

        int Caustics::init() {
            shader = Shader("src/renderer/shader/caustics/caustics.vert", "src/renderer/shader/caustics/caustics.frag");
            write2photonVBOCS = ComputeShader("src/renderer/shader/caustics/write2photonVBO.comp");

            // FBO
            {
            glGenFramebuffers(1, &FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->terminatePositionTexture, 0);
            utils::bindColorAttachment(m_info->validTexture, 1);
            utils::bindColorAttachment(m_info->redPositionTexture, 2);
            utils::bindColorAttachment(m_info->greenPositionTexture, 3);
            utils::bindColorAttachment(m_info->bluePositionTexture, 4);

            utils::setDrawbuffersLayout(4);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "Caustics framebuffer is not complete" << std::endl;
                return -1;
            }
            }

            return 0;
        }

        int Caustics::write2photonVBO() {
            write2photonVBOCS.use();
            write2photonVBOCS.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 20, m_info->photonSSBO);

            // white
            utils::bindTextureWithLayer0(m_info->terminatePositionTexture, 7, GL_RGBA32F, GL_READ_ONLY);
            write2photonVBOCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            utils::copySSBO2VBO(m_info->photonSSBO, m_info->photonPositionVBO, m_info->width * m_info->height * sizeof(glm::vec4));

            // red
            utils::bindTextureWithLayer0(m_info->redPositionTexture, 7, GL_RGBA32F, GL_READ_ONLY);
            write2photonVBOCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            utils::copySSBO2VBO(m_info->photonSSBO, m_info->redPhotonPositionVBO, m_info->width * m_info->height * sizeof(glm::vec4));

            // green
            utils::bindTextureWithLayer0(m_info->greenPositionTexture, 7, GL_RGBA32F, GL_READ_ONLY);
            write2photonVBOCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            utils::copySSBO2VBO(m_info->photonSSBO, m_info->greenPhotonPositionVBO, m_info->width * m_info->height * sizeof(glm::vec4));

            // blue
            utils::bindTextureWithLayer0(m_info->bluePositionTexture, 7, GL_RGBA32F, GL_READ_ONLY);
            write2photonVBOCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            utils::copySSBO2VBO(m_info->photonSSBO, m_info->bluePhotonPositionVBO, m_info->width * m_info->height * sizeof(glm::vec4));

            return 0;
        }

        int Caustics::render() {
            // prepare
            m_info->setViewport();
            m_scene->render();
            m_fluid->render();

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);


            shader.use();
            // uniforms
            // MVP transform
            shader.setMat4("uView", m_info->view());
            shader.setMat4("uProjection", m_info->projection());
            // other parameters
            shader.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            shader.setFloat("uRefractionRatio", 1.0f / 1.33f);
            shader.setFloat("uRedRefractionRatio", 1.0f / 1.31f);
            shader.setFloat("uGreenRefractionRatio", 1.0f / 1.33f);
            shader.setFloat("uBlueRefractionRatio", 1.0f / 1.35f);
            shader.setVec3("uLightPosition", m_info->camera.Position);
            // textures
            utils::bindTexture2D(shader, "uSceneDepthTexture", m_scene->m_info->depthTexture, 0);
            utils::bindTexture2D(shader, "uScenePositionTexture", m_scene->m_info->positionTexture, 1);
            utils::bindTexture2D(shader, "uSceneValidTexture", m_scene->m_info->validTexture, 2);
            utils::bindTexture2D(shader, "uFluidDepthTexture", m_fluid->m_info->depthTexture, 3);
            utils::bindTexture2D(shader, "uFluidNormalTexture", m_fluid->m_info->normalTexture, 4);
            utils::bindTexture2D(shader, "uFluidPositionTexture", m_fluid->m_info->positionTexture, 5);
            utils::bindTexture2D(shader, "uFluidValidTexture", m_fluid->m_info->validTexture, 6);

            utils::drawScreenQuad();
            glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            write2photonVBO();

            return 0;
        }

        int Caustics::terminate() {
            glDeleteFramebuffers(1, &FBO);

            return 0;
        }
    }
}