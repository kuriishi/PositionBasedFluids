#include "renderer.hpp"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <memory>

#include "../common/common.hpp"
#include "../common/shader.hpp"
#include "../common/performance_log.hpp"
#include "../simulator/simulator.hpp"
#include "scene.hpp"
#include "fluid.hpp"
#include "window.hpp"
#include "utils.hpp"
#include "info.hpp"

namespace renderer {
    bool enableCaustics = true;
    RenderMode renderMode = FLUID_AND_SCENE;

    // performance log
    const unsigned int QUERY_START_INDEX = common::SIMULATE_TIME_QUERY_COUNT + 2;

    Shader fluidAndSceneShader;
    Shader photonTerminatePositionShader;

    Camera causticsCamera;

    Renderer::Renderer(): camera(window::camera), width(window::SCR_WIDTH), height(window::SCR_HEIGHT) {
        window::windowInit();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        causticsCamera = Camera(glm::vec3(-3.0, 3.0, -3.0));
        const int CAUSTICS_RADIUS = 2048;
        m_caustics = std::make_shared<caustics::Caustics>(causticsCamera, CAUSTICS_RADIUS, CAUSTICS_RADIUS);

        m_sceneWithCaustics = std::make_shared<sceneWithCaustics::SceneWithCaustics>(camera, width, height, m_caustics->m_info);
        m_scene = std::make_shared<scene::Scene>(camera, width, height);
        m_fluid = std::make_shared<fluid::Fluid>(camera, width, height, m_scene->m_info);

        init();
    }

    int Renderer::init() {
        fluidAndSceneShader = Shader("src/renderer/shader/renderer.vert", "src/renderer/shader/renderer.frag");
        photonTerminatePositionShader = Shader("src/renderer/shader/renderer.vert", "src/renderer/shader/rendererCausticsTerminatePosition.frag");

        return 0;
    }

    Renderer::~Renderer() {
        glDeleteProgram(fluidAndSceneShader.ID);
        glDeleteProgram(photonTerminatePositionShader.ID);
    }

    int Renderer::render() {
        if (renderMode == FLUID_AND_SCENE) {
            renderFluidAndScene();
        }
        else if (renderMode == PHOTON_TERMINATE_POSITION) {
            renderCausticsTerminatePosition();
        }

        return 0;
    }

    int Renderer::renderCausticsTerminatePosition() {
        m_caustics->render();

        {
            for (int i = 0; i <= 5; i++) {
                common::queryTime(QUERY_START_INDEX + i);
            }
        }

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

        photonTerminatePositionShader.use();
        utils::bindTexture2D(photonTerminatePositionShader, "uCausticsTerminatePositionTexture", m_caustics->m_info->terminatePositionTexture, 0);
        utils::drawScreenQuad();

        {
            common::queryTime(QUERY_START_INDEX + 6);
        }

        return 0;
    }

    int Renderer::renderFluidAndScene() {
        if (enableCaustics) {
            m_fluid->setSceneInfo(m_sceneWithCaustics->m_info);
            m_caustics->render();
            m_sceneWithCaustics->render();
        }
        else {
            m_fluid->setSceneInfo(m_scene->m_info);
            m_scene->render();
        }

        {
        common::queryTime(QUERY_START_INDEX + 0);
        }

        m_fluid->renderPrepare();

        {
        common::queryTime(QUERY_START_INDEX + 5);
        }

        m_fluid->render(false);

        glViewport(0, 0, width, height);
        // render
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

        fluidAndSceneShader.use();

        if (enableCaustics) {
            utils::bindTexture2D(fluidAndSceneShader, "uSceneColorTexture", m_sceneWithCaustics->m_info->colorTexture, 0);
            utils::bindTexture2D(fluidAndSceneShader, "uSceneDepthTexture", m_sceneWithCaustics->m_info->depthTexture, 1);
        }
        else {
            utils::bindTexture2D(fluidAndSceneShader, "uSceneColorTexture", m_scene->m_info->colorTexture, 0);
            utils::bindTexture2D(fluidAndSceneShader, "uSceneDepthTexture", m_scene->m_info->depthTexture, 1);
        }

        utils::bindTexture2D(fluidAndSceneShader, "uFluidColorTexture", m_fluid->m_info->colorTexture, 2);
        utils::bindTexture2D(fluidAndSceneShader, "uFluidDepthTexture", m_fluid->m_info->depthTexture, 3);
        utils::bindTexture2D(fluidAndSceneShader, "uFluidValidTexture", m_fluid->m_info->validTexture, 4);

        utils::drawScreenQuad();

        {
        common::queryTime(QUERY_START_INDEX + 6);
        }

        return 0;   
    }

}