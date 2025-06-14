#include "info.hpp"

#include "utils.hpp"

namespace renderer {
    namespace info {
        // Info
        Info::Info(Camera& camera, unsigned int width, unsigned int height)
            : camera(camera), width(width), height(height) {
        }

        Info::~Info() {
        }

        glm::mat4 Info::view() {
            return camera.GetViewMatrix();
        }

        glm::mat4 Info::projection() {
            return camera.GetProjectionMatrix(width, height);
        }

        glm::vec3 Info::position() {
            return camera.Position;
        }

        void Info::setViewport() {
            glViewport(0, 0, width, height);
        }

        // Scene Info
        SceneInfo::SceneInfo(Camera& camera, unsigned int width, unsigned int height)
            : Info(camera, width, height) {
            colorTexture = utils::generateTextureRGBA32F(width, height);
            validTexture = utils::generateTextureR8I(width, height);
            depthTexture = utils::generateTextureDepth32F(width, height);
            positionTexture = utils::generateTextureRGBA32F(width, height);
            normalTexture = utils::generateTextureRGBA32F(width, height);

            skyboxColorTexture = utils::generateTextureRGBA32F(width, height);
            otherSceneColorTexture = utils::generateTextureRGBA32F(width, height);
        }

        SceneInfo::~SceneInfo() {
            glDeleteTextures(1, &colorTexture);
            glDeleteTextures(1, &validTexture);
            glDeleteTextures(1, &depthTexture);
            glDeleteTextures(1, &positionTexture);
            glDeleteTextures(1, &normalTexture);

            glDeleteTextures(1, &skyboxColorTexture);
            glDeleteTextures(1, &otherSceneColorTexture);
        }

        // Fluid Info
        FluidInfo::FluidInfo(Camera& camera, unsigned int width, unsigned int height)
            : Info(camera, width, height) {
            colorTexture = utils::generateTextureRGBA32F(width, height);
            validTexture = utils::generateTextureR8I(width, height);
            positionTexture = utils::generateTextureRGBA32F(width, height);
            normalTexture = utils::generateTextureRGBA32F(width, height);
            depthTexture = utils::generateTextureDepth32F(width, height);
            smoothedDepthTexture = utils::generateTextureR32F(width, height);
        }

        FluidInfo::~FluidInfo() {
            glDeleteTextures(1, &colorTexture);
            glDeleteTextures(1, &validTexture);
            glDeleteTextures(1, &positionTexture);
            glDeleteTextures(1, &normalTexture);
            glDeleteTextures(1, &depthTexture);
            glDeleteTextures(1, &smoothedDepthTexture);
        }

        // Caustics Info
        CausticsInfo::CausticsInfo(Camera& camera, unsigned int width, unsigned int height)
            : Info(camera, width, height) {
            depthTexture = utils::generateTextureDepth32F(width, height);
            validTexture = utils::generateTextureR8I(width, height);
            terminatePositionTexture = utils::generateTextureRGBA32F(width, height);
            redPositionTexture = utils::generateTextureRGBA32F(width, height);
            greenPositionTexture = utils::generateTextureRGBA32F(width, height);
            bluePositionTexture = utils::generateTextureRGBA32F(width, height);

            // vertex buffer and aid SSBO
            photonCount = width * height;
            glGenBuffers(1, &photonPositionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, photonPositionVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * photonCount, NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &redPhotonPositionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, redPhotonPositionVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * photonCount, NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &greenPhotonPositionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, greenPhotonPositionVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * photonCount, NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &bluePhotonPositionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, bluePhotonPositionVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * photonCount, NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &photonSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, photonSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * photonCount, NULL, GL_DYNAMIC_COPY);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        CausticsInfo::~CausticsInfo() {
            glDeleteTextures(1, &depthTexture);
            glDeleteTextures(1, &validTexture);
            glDeleteTextures(1, &terminatePositionTexture);
            glDeleteTextures(1, &redPositionTexture);
            glDeleteTextures(1, &greenPositionTexture);
            glDeleteTextures(1, &bluePositionTexture);

            glDeleteBuffers(1, &photonPositionVBO);
            glDeleteBuffers(1, &redPhotonPositionVBO);
            glDeleteBuffers(1, &greenPhotonPositionVBO);
            glDeleteBuffers(1, &bluePhotonPositionVBO);
            glDeleteBuffers(1, &photonSSBO);
        }
    }
}
