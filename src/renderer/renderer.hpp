#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

#include "../common/common.hpp"
#include "scene.hpp"
#include "sceneWithCaustics.hpp"
#include "fluid.hpp"
#include "caustics.hpp"
#include "camera.hpp"

namespace renderer {
    extern bool enableCaustics;

    enum RenderMode {
        FLUID_AND_SCENE,
        PHOTON_TERMINATE_POSITION,
    };
    extern RenderMode renderMode;

    class Renderer {
        public:
            Renderer();
            ~Renderer();

            int render();
        private:
            Camera& camera;
            unsigned int width;
            unsigned int height;

            std::shared_ptr<caustics::Caustics> m_caustics;
            std::shared_ptr<sceneWithCaustics::SceneWithCaustics> m_sceneWithCaustics;
            std::shared_ptr<scene::Scene> m_scene;
            std::shared_ptr<fluid::Fluid> m_fluid;

            int init();

            int renderCausticsTerminatePosition();
            int renderFluidAndScene();
    };

}
