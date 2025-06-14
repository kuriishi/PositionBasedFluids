#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>

#include "camera.hpp"
#include "scene.hpp"
#include "caustics.hpp"
#include "info.hpp"

namespace renderer {
    namespace sceneWithCaustics {
        extern float uPhotonEnergy;
        extern float uPhotonSize;
        extern int blurCount;

        class SceneWithCaustics {
            public:
                SceneWithCaustics(Camera& camera, unsigned int width, unsigned int height, std::shared_ptr<renderer::info::CausticsInfo> causticsInfo);
                ~SceneWithCaustics();

                int render();

                std::shared_ptr<renderer::info::SceneInfo> m_info;
            
            private:
                std::shared_ptr<renderer::info::CausticsInfo> m_causticsInfo;
                std::shared_ptr<renderer::scene::Scene> m_scene;

                GLuint FBO;

                GLuint causticsFBO;
                GLuint causticsColorTexture;

                GLuint causticsPointVAO;
                GLuint causticsPointRedVAO;
                GLuint causticsPointGreenVAO;
                GLuint causticsPointBlueVAO;
                GLuint causticsTriangleVAO;

                GLuint causticsBlurTexture;

                void prepare();
                void clean();
                // 0: white; 1: red; 2: green; 3: blue
                int renderCausticsAsPoint(int flag);

                int discardTooFewPhotons(int count);
                int spatialBlur();
            
                int init();
        };
    }
}
