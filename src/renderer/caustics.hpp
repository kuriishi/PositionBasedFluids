#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>

#include "../common/common.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "fluid.hpp"

namespace renderer {
    namespace caustics {
        class Caustics {
            public:
                Caustics(Camera& camera, unsigned int width, unsigned int height);
                ~Caustics();

                // refraction only
                int render();

                std::shared_ptr<renderer::info::CausticsInfo> m_info;

            private:
                GLuint FBO;
                std::shared_ptr<scene::Scene> m_scene;
                std::shared_ptr<fluid::Fluid> m_fluid;

                int write2photonVBO();

                int init();
                int terminate();
        };
    }
}