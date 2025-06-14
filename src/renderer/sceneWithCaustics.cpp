#include "sceneWithCaustics.hpp"

#include "utils.hpp"

namespace renderer {
    namespace sceneWithCaustics {
        Shader shader;
        Shader causticsPointShader;
        Shader causticsTriangleShader;
        Shader causticsPixelShader;

        ComputeShader discardTooFewPhotonsCS;
        ComputeShader spatialBlurCS;

        float uPhotonEnergy = 0.002f;
        float uPhotonSize = 0.05f;
        int blurCount = 3;

        SceneWithCaustics::SceneWithCaustics(Camera& camera, unsigned int width, unsigned int height, std::shared_ptr<renderer::info::CausticsInfo> causticsInfo) {
            m_causticsInfo = causticsInfo;
            m_scene = std::make_shared<renderer::scene::Scene>(camera, width, height);

            m_info = std::make_shared<renderer::info::SceneInfo>(camera, width, height);

            init();
        }

        SceneWithCaustics::~SceneWithCaustics() {
            glDeleteFramebuffers(1, &FBO);
        }

        int SceneWithCaustics::init() {
            // main shader
            shader = Shader("src/renderer/shader/sceneWithCaustics/sceneWithCaustics.vert", "src/renderer/shader/sceneWithCaustics/sceneWithCaustics.frag");

            // main framebuffer
            glGenFramebuffers(1, &FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->colorTexture, 0);
            utils::bindColorAttachment(m_info->validTexture, 1);
            utils::bindColorAttachment(m_info->positionTexture, 2);
            utils::bindColorAttachment(m_info->normalTexture, 3);

            utils::setDrawbuffersLayout(3);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "SceneWithCaustics::init(): main framebuffer is not complete" << std::endl;
                return -1;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            m_info->skyboxTexture = m_scene->m_info->skyboxTexture;

            // caustics framebuffer
            glGenFramebuffers(1, &causticsFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, causticsFBO);
            causticsColorTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);
            utils::bindColorAttachment(causticsColorTexture, 0);
            utils::bindDepthAttachment(m_info->depthTexture);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "SceneWithCaustics::init(): caustics framebuffer is not complete" << std::endl;
                return -1;
            }

            // caustics as points
            causticsPointShader = Shader("src/renderer/shader/sceneWithCaustics/causticsPoint.vert", "src/renderer/shader/sceneWithCaustics/causticsPoint.frag");

            glGenVertexArrays(1, &causticsPointVAO);
            glBindVertexArray(causticsPointVAO);

            glBindBuffer(GL_ARRAY_BUFFER, m_causticsInfo->photonPositionVBO);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // red
            glGenVertexArrays(1, &causticsPointRedVAO);
            glBindVertexArray(causticsPointRedVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_causticsInfo->redPhotonPositionVBO);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // green
            glGenVertexArrays(1, &causticsPointGreenVAO);
            glBindVertexArray(causticsPointGreenVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_causticsInfo->greenPhotonPositionVBO);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // blue
            glGenVertexArrays(1, &causticsPointBlueVAO);
            glBindVertexArray(causticsPointBlueVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_causticsInfo->bluePhotonPositionVBO);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // discard too few photons
            discardTooFewPhotonsCS = ComputeShader("src/renderer/shader/sceneWithCaustics/discardTooFewPhotons.comp");

            // blur
            spatialBlurCS = ComputeShader("src/renderer/shader/sceneWithCaustics/spatialBlur.comp");
            causticsBlurTexture = utils::generateTextureRGBA32F(m_info->width, m_info->height);

            return 0;
        }
    
        void SceneWithCaustics::prepare() {
            glBindFramebuffer(GL_FRAMEBUFFER, causticsFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);

            glEnable(GL_PROGRAM_POINT_SIZE);

            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
        }

        void SceneWithCaustics::clean() {
            glDisable(GL_BLEND);
            glDisable(GL_PROGRAM_POINT_SIZE);
            glDepthMask(GL_TRUE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        int SceneWithCaustics::renderCausticsAsPoint(int flag) {
            causticsPointShader.use();
            // uniforms
            // MVP transforms
            glm::mat4 uView = m_info->view();
            causticsPointShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            causticsPointShader.setMat4("uProjection", uProjection);
            // other parameters
            causticsPointShader.setFloat("uPhotonSize", uPhotonSize);
            causticsPointShader.setInt("uFlag", flag);
            causticsPointShader.setFloat("uPhotonEnergy", uPhotonEnergy);

            if (flag == 0) {
                glBindVertexArray(causticsPointVAO);
            } else if (flag == 1) {
                glBindVertexArray(causticsPointRedVAO);
            } else if (flag == 2) {
                glBindVertexArray(causticsPointGreenVAO);
            } else if (flag == 3) {
                glBindVertexArray(causticsPointBlueVAO);
            }
            glDrawArrays(GL_POINTS, 0, m_causticsInfo->photonCount);
            glBindVertexArray(0);

            return 0;
        }

        int SceneWithCaustics::discardTooFewPhotons(int count) {
            discardTooFewPhotonsCS.use();
            // uniforms
            // textures
            utils::bindTextureWithLayer0(causticsColorTexture, 0, GL_RGBA32F, GL_READ_WRITE);
            // other parameters
            discardTooFewPhotonsCS.setIvec2("uCausticsTextureSize", glm::ivec2(m_info->width, m_info->height));
            discardTooFewPhotonsCS.setFloat("uPhotonEnergy", uPhotonEnergy);
            discardTooFewPhotonsCS.setInt("uMinPhotons", count);

            discardTooFewPhotonsCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return 0;
        }

        int SceneWithCaustics::spatialBlur() {
            spatialBlurCS.use();
            // uniforms
            spatialBlurCS.setIvec2("uTextureSize", glm::ivec2(m_info->width, m_info->height));

            // horizontal
            spatialBlurCS.setBool("uHorizontal", true);
            utils::bindTextureWithLayer0(causticsColorTexture, 1, GL_RGBA32F, GL_READ_WRITE);
            utils::bindTextureWithLayer0(causticsBlurTexture, 2, GL_RGBA32F, GL_WRITE_ONLY);
            spatialBlurCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // vertical
            spatialBlurCS.setBool("uHorizontal", false);
            utils::bindTextureWithLayer0(causticsBlurTexture, 1, GL_RGBA32F, GL_READ_WRITE);
            utils::bindTextureWithLayer0(causticsColorTexture, 2, GL_RGBA32F, GL_WRITE_ONLY);
            spatialBlurCS.dispatchCompute(m_info->width * m_info->height);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return 0;
        }

        int SceneWithCaustics::render() {
            m_info->setViewport();
            m_scene->render();

            prepare();
            for (int i = 1; i <= 3; i++) {
                renderCausticsAsPoint(i);
            }
            clean();

            discardTooFewPhotons(9);
            for (int i = 0; i < blurCount; i++) {
                spatialBlur();
            }
            discardTooFewPhotons(9);

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);

            shader.use();
            // textures
            utils::bindTexture2D(shader, "uCausticsTexture", causticsColorTexture, 0);
            utils::bindTexture2D(shader, "uSceneColorTexture", m_scene->m_info->colorTexture, 1);
            utils::bindTexture2D(shader, "uSceneValidTexture", m_scene->m_info->validTexture, 2);
            utils::bindTexture2D(shader, "uScenePositionTexture", m_scene->m_info->positionTexture, 3);
            utils::bindTexture2D(shader, "uSceneNormalTexture", m_scene->m_info->normalTexture, 4);
            utils::bindTexture2D(shader, "uSceneDepthTexture", m_scene->m_info->depthTexture, 5);
            // shadow
            utils::bindTexture2D(shader, "uCausticsDepthTexture", m_causticsInfo->depthTexture, 6);
            // other uniforms
            shader.setIvec2("uCausticsResolution", glm::ivec2(m_causticsInfo->width, m_causticsInfo->height));
            shader.setVec3("uLightPosition", m_causticsInfo->position());
            shader.setMat4("uCausticsView", m_causticsInfo->view());
            shader.setMat4("uCausticsProjection", m_causticsInfo->projection());
            shader.setFloat("uEpsilon", 0.001f);

            utils::drawScreenQuad();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

    }
}
