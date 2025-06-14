#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.hpp"

namespace renderer {
    namespace info {
        class Info {
            public:
                Info(Camera& camera, unsigned int width, unsigned int height);
                ~Info();

                glm::mat4 view();
                glm::mat4 projection();
                glm::vec3 position();

                void setViewport();

                Camera& camera;
                unsigned int width;
                unsigned int height;
        };

        class SceneInfo : public Info {
            public:
                SceneInfo(Camera& camera, unsigned int width, unsigned int height);
                ~SceneInfo();

                // off screen information used later
                GLuint colorTexture;
                GLuint validTexture;
                GLuint depthTexture;
                GLuint positionTexture;
                GLuint normalTexture;

                // classification color
                GLuint skyboxColorTexture;
                GLuint otherSceneColorTexture;
                
                // others
                GLuint skyboxTexture;
        };

        class FluidInfo : public Info {
            public:
                FluidInfo(Camera& camera, unsigned int width, unsigned int height);
                ~FluidInfo();

                GLuint colorTexture;
                GLuint validTexture;
                GLuint positionTexture;
                GLuint normalTexture;
                GLuint depthTexture;
                GLuint smoothedDepthTexture;
        };

        class CausticsInfo: public Info {
            public:
                CausticsInfo(Camera& camera, unsigned int width, unsigned int height);
                ~CausticsInfo();

                GLuint depthTexture;
                GLuint validTexture;
                GLuint terminatePositionTexture;
                GLuint redPositionTexture;
                GLuint greenPositionTexture;
                GLuint bluePositionTexture;

                // vertex buffer and aid SSBO
                unsigned int photonCount;
                GLuint photonPositionVBO;
                GLuint redPhotonPositionVBO;
                GLuint greenPhotonPositionVBO;
                GLuint bluePhotonPositionVBO;
                GLuint photonSSBO;

        };
    }
}
