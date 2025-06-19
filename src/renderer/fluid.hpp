#pragma once

#include <memory>

#include "../common/shader.hpp"
#include "../common/compute_shader.hpp"
#include "camera.hpp"
#include "info.hpp"

namespace renderer {
    namespace fluid {

        class Fluid {
            public:
                Fluid(Camera& camera, unsigned int width, unsigned int height, std::shared_ptr<renderer::info::SceneInfo> sceneInfo);
                ~Fluid();

                int renderPrepare();
                int render(bool prepare = true);
                int setSceneInfo(std::shared_ptr<renderer::info::SceneInfo> sceneInfo);

                std::shared_ptr<renderer::info::FluidInfo> m_info;

            private:
                std::shared_ptr<renderer::info::SceneInfo> m_sceneInfo;

                GLuint FBO;
                GLuint VAO;
                GLuint VBO;
                GLuint depthFBO;
                GLuint thicknessFBO;

                GLuint smoothedDepthAidTexture;
                GLuint thicknessTexture;
                GLuint normalViewSpaceTexture;
                GLuint repairedNormalViewSpaceTexture;

                GLuint densityVBO;

                int init();
                int clear();
                int renderDepthTexture();
                int renderThicknessTexture();
                int smoothDepthTexture(int kernelRadius);
                int computeNormalTexture();
                int fixInvalidNormals();

                int renderFluid();
                int renderDepth();
                int renderThickness();
                int renderNormal();
                int renderParticle();

                GLuint foamFBO;
                GLuint foamTexture;
                GLuint erodedFoamTexture;
                GLuint edgeTexture;

                int renderCartoon();
                int renderFoamTexture();
                int renderFoam();
                int erodeFoamTexture();
                int computeEdgeTexture();
                int renderEdge();

                int copyParticleAttribute();
        };
    }
}