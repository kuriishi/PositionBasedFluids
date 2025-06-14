#pragma once

#include "glad/glad.h"

#include <memory>

#include "camera.hpp"
#include "info.hpp"

namespace renderer {
    namespace scene {


        class Scene {
            public:
                Scene(Camera& camera, unsigned int width, unsigned int height);
                ~Scene();
                
                // // off screen information used later
                // GLuint colorTexture;
                // GLuint validTexture;
                // GLuint depthTexture;
                // GLuint positionTexture;
                // GLuint normalTexture;

                // // classification color
                // GLuint skyboxColorTexture;
                // GLuint floorColorTexture;
                
                // // others
                // GLuint skyboxTexture;

                int render();

                std::shared_ptr<renderer::info::SceneInfo> m_info;
            
            private:
                // Camera& camera;
                // unsigned int width;
                // unsigned int height;

                GLuint FBO;

                GLuint skyboxFBO;
                GLuint skyboxVAO;
                GLuint skyboxVBO;

                GLuint floorFBO;
                GLuint floorVAO;
                GLuint floorVBO;
                GLuint floorTexture;

                int clear();

                int initSkybox();
                int renderSkybox();
                int terminateSkybox();

                int initFloor();
                int renderFloor();
                int terminateFloor();

                int init();
                int terminate();
        };
    }
}
