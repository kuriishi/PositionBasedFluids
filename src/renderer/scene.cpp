#include "scene.hpp"


#include "../../include/stb_image.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <vector>
#include <string>

#include "../common/shader.hpp"
#include "../common/compute_shader.hpp"
#include "utils.hpp"

namespace renderer {
    namespace scene {
        ComputeShader clearCS;
        Shader skyboxShader;
        Shader floorShader;
        Shader shader;

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
        const float FLOOR_SIZE = 16.0;

        // scene
        int Scene::initSkybox() {
            skyboxShader = Shader("src/renderer/shader/scene/skybox/skybox.vert", "src/renderer/shader/scene/skybox/skybox.frag");

            stbi_set_flip_vertically_on_load(false);
            m_info->skyboxTexture = utils::loadCubemap(skyboxFaces);

            // framebuffer
            {
            glGenFramebuffers(1, &skyboxFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->skyboxColorTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Skybox framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // vertex array and buffer
            {
            glGenVertexArrays(1, &skyboxVAO);
            glGenBuffers(1, &skyboxVBO);
        
            glBindVertexArray(skyboxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
            glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
            }

            return 0;
        }

        int Scene::renderSkybox() {
            glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);

            skyboxShader.use();

            glm::mat4 uView = glm::mat4(glm::mat3(m_info->view()));
            skyboxShader.setMat4("uView", uView);

            glm::mat4 uProjection = m_info->projection();
            skyboxShader.setMat4("uProjection", uProjection);

            utils::bindTextureCubeMap(skyboxShader, "uSkyboxTexture", m_info->skyboxTexture, 0);

            glBindVertexArray(skyboxVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);

            glDepthFunc(GL_LESS);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return 0;
        }

        int Scene::terminateSkybox() {
            glDeleteVertexArrays(1, &skyboxVAO);
            glDeleteBuffers(1, &skyboxVBO);
            glDeleteProgram(skyboxShader.ID);

            glDeleteTextures(1, &m_info->skyboxTexture);

            skyboxVertices.clear();
            skyboxFaces.clear();

            return 0;
        }
    
        int Scene::initFloor() {
            floorShader = Shader("src/renderer/shader/scene/floor/floor.vert", "src/renderer/shader/scene/floor/floor.frag");

            stbi_set_flip_vertically_on_load(false);
            floorTexture = utils::loadTexture("resource/floor.jpg");

            // framebuffer
            {
            glGenFramebuffers(1, &floorFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, floorFBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->otherSceneColorTexture, 0);
            utils::bindColorAttachment(m_info->validTexture, 1);
            utils::bindColorAttachment(m_info->positionTexture, 2);
            utils::bindColorAttachment(m_info->normalTexture, 3);
            utils::setDrawbuffersLayout(3);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Floor framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // vertex array and buffer
            {
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
            }

            return 0;
        }

        int Scene::renderFloor() {
            glBindFramebuffer(GL_FRAMEBUFFER, floorFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);

            floorShader.use();
            // MVP transform
            glm::mat4 uModel = glm::mat4(1.0f);
            uModel = glm::scale(uModel, glm::vec3(FLOOR_SIZE, 1.0f, FLOOR_SIZE));
            #ifdef eGPU
            uModel = glm::translate(uModel, glm::vec3(0.0f, -1.5f, 0.0f));
            #else
            uModel = glm::translate(uModel, glm::vec3(0.0f, -0.25f, 0.0f));
            #endif
            // uModel = glm::translate(uModel, glm::vec3(0.0f, -1.0f, 0.0f));
            floorShader.setMat4("uModel", uModel);
            glm::mat4 uView = m_info->view();
            floorShader.setMat4("uView", uView);
            glm::mat4 uProjection = m_info->projection();
            floorShader.setMat4("uProjection", uProjection);
            glm::mat4 uModelTranspose = glm::transpose(uModel);
            floorShader.setMat4("uModelTranspose", uModelTranspose);
            // textures
            utils::bindTexture2D(floorShader, "uFloorTexture", floorTexture, 0);
            // light
            floorShader.setVec3("uLight.position", glm::vec3(3.0f, 3.0f, 3.0f));
            floorShader.setVec3("uLight.intensity", glm::vec3(1.0f, 1.0f, 1.0f));

            glBindVertexArray(floorVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);   

            return 0;
        }

        int Scene::terminateFloor() {
            glDeleteVertexArrays(1, &floorVAO);
            glDeleteBuffers(1, &floorVBO);
            glDeleteTextures(1, &floorTexture);
            glDeleteProgram(floorShader.ID);

            floorVertices.clear();

            return 0;
        }

        int Scene::init() {
            clearCS = ComputeShader("src/renderer/shader/scene/clear.comp");
            shader = Shader("src/renderer/shader/scene/scene.vert", "src/renderer/shader/scene/scene.frag");

            // framebuffer
            {
            glGenFramebuffers(1, &FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            utils::bindDepthAttachment(m_info->depthTexture);
            utils::bindColorAttachment(m_info->colorTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Scene off screen framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // vertex array and buffer
            {
            }

            initSkybox();
            initFloor();

            return 0;
        }

        int Scene::clear() {
            clearCS.use();
            clearCS.setIvec2("uScreenSize", glm::ivec2(m_info->width, m_info->height));
            clearCS.dispatchCompute(m_info->width * m_info->height);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            return 0;
        }

        int Scene::render() {
            m_info->setViewport();

            // prepare
            clear();
            renderSkybox();
            renderFloor();

            // off screen render
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_FALSE);

            shader.use();
            utils::bindTexture2D(shader, "uOtherSceneColorTexture", m_info->otherSceneColorTexture, 0);
            utils::bindTexture2D(shader, "uSkyboxColorTexture", m_info->skyboxColorTexture, 1);
            utils::bindTexture2D(shader, "uValidTexture", m_info->validTexture, 2);

            utils::drawScreenQuad();

            glDepthMask(GL_TRUE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            return 0;
        }

        int Scene::terminate() {
            terminateSkybox();
            terminateFloor();

            glDeleteFramebuffers(1, &skyboxFBO);
            glDeleteFramebuffers(1, &floorFBO);
            

            return 0;
        }

        Scene::Scene(Camera& camera, unsigned int width, unsigned int height) 
                    : m_info(std::make_shared<renderer::info::SceneInfo>(camera, width, height)) {
            init();
        }

        Scene::~Scene() {
            terminate();
        }

    }
}
