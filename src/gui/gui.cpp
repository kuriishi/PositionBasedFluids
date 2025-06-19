#include "gui.hpp"

#include "../../include/imgui/imgui.h"
#include "../../include/imgui/imgui_impl_glfw.h"
#include "../../include/imgui/imgui_impl_opengl3.h"

#include "../renderer/window.hpp"
#include "../common/performance_log.hpp"
#include "../renderer/renderer.hpp"
#include "../renderer/scene.hpp"
#include "../simulator/simulator.hpp"

#include "../renderer/parameter.hpp"

namespace gui {
    int guiInit() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.FontGlobalScale = 2.0f;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(renderer::window::window, true);
        ImGui_ImplOpenGL3_Init(GLSL_VERSION);

        glFinish();

        return 0;
    }

    int guiRender() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!common::hideGUI) {
            guiDraw();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        int display_w, display_h;
        glfwGetFramebufferSize(renderer::window::window, &display_w, &display_h);

        return 0;
    }

    int guiTerminate() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glFinish();

        return 0;
    }

    int guiDraw() {
        static bool showControlPanel = true;
        static bool showPerformanceMonitor = true;
        static bool showSimulatePerformance = true;
        static bool showRenderPerformance = true;

        ImGui::Begin("Panel Visibility");
        ImGui::Checkbox("Control Panel", &showControlPanel);
        ImGui::Checkbox("Performance Monitor", &showPerformanceMonitor);
        ImGui::Checkbox("Render Performance", &showRenderPerformance);
        ImGui::Checkbox("Simulate Performance", &showSimulatePerformance);
        ImGui::End();

        // Control Panel
        if (showControlPanel) {
            ImGui::Begin("Control Panel");

            // render
            {
                if (ImGui::RadioButton("Fluid and Scene", renderer::renderMode == renderer::RenderMode::FLUID_AND_SCENE)) 
                    renderer::renderMode = renderer::RenderMode::FLUID_AND_SCENE;
                if (ImGui::RadioButton("Photon Terminate Position", renderer::renderMode == renderer::RenderMode::PHOTON_TERMINATE_POSITION)) 
                    renderer::renderMode = renderer::RenderMode::PHOTON_TERMINATE_POSITION;

                ImGui::Separator();

                ImGui::Checkbox("Smooth Depth", &renderer::fluid::enableSmoothDepth);
                ImGui::SliderInt("Smooth Iteration", &renderer::fluid::smoothIteration, 1, 256);
                ImGui::Checkbox("Separate Bilateral Filter", &renderer::fluid::separateBilateralFilter);
                static float particleRadiusScaler = renderer::fluid::particleRadiusScaler * 0.1f;
                ImGui::SliderFloat("Particle Radius Scaler", &particleRadiusScaler, 0.0f, 1.0f);
                renderer::fluid::particleRadiusScaler = particleRadiusScaler * 10.0f;
                static float minimumDensityScaler = renderer::fluid::minimumDensityScaler * 0.5f;
                ImGui::SliderFloat("Minimum Density Scaler", &minimumDensityScaler, 0.0f, 1.0f);
                renderer::fluid::minimumDensityScaler = minimumDensityScaler * 2.0f;

                if (renderer::fluid::displayMode == renderer::fluid::DisplayMode::CARTOON || renderer::fluid::displayMode == renderer::fluid::DisplayMode::FOAM) {
                    static float foamDensityScaler = renderer::fluid::foamDensityScaler * 0.5f;
                    ImGui::SliderFloat("Foam Density Scaler", &foamDensityScaler, 0.0f, 1.0f);
                    renderer::fluid::foamDensityScaler = foamDensityScaler * 2.0f;
                    if (renderer::fluid::foamDensityScaler < renderer::fluid::minimumDensityScaler) {
                        renderer::fluid::foamDensityScaler = renderer::fluid::minimumDensityScaler;
                    }

                    ImGui::Separator();

                    ImGui::SliderInt("Foam Erode Kernel Radius", &renderer::fluid::foamErodeKernelRadius, 1, 16);
                    ImGui::SliderInt("Foam Erode Minimun Neighbor Count", &renderer::fluid::foamErodeMinimunNeighborCount, 1, 128);
                }


                ImGui::Separator();

                if (renderer::renderMode == renderer::RenderMode::FLUID_AND_SCENE) {
                    if (ImGui::RadioButton("Fluid", renderer::fluid::displayMode == renderer::fluid::DisplayMode::FLUID)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::FLUID;
                    if (ImGui::RadioButton("Cartoon", renderer::fluid::displayMode == renderer::fluid::DisplayMode::CARTOON)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::CARTOON;
                    
                    if (renderer::fluid::displayMode == renderer::fluid::DisplayMode::CARTOON || renderer::fluid::displayMode == renderer::fluid::DisplayMode::FOAM) {
                        if (ImGui::RadioButton("Foam", renderer::fluid::displayMode == renderer::fluid::DisplayMode::FOAM)) 
                            renderer::fluid::displayMode = renderer::fluid::DisplayMode::FOAM;
                    }

                    if (ImGui::RadioButton("Thickness", renderer::fluid::displayMode == renderer::fluid::DisplayMode::THICKNESS)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::THICKNESS;
                    if (ImGui::RadioButton("Normal", renderer::fluid::displayMode == renderer::fluid::DisplayMode::NORMAL)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::NORMAL;
                    if (ImGui::RadioButton("Depth", renderer::fluid::displayMode == renderer::fluid::DisplayMode::DEPTH)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::DEPTH;
                    if (ImGui::RadioButton("Particle", renderer::fluid::displayMode == renderer::fluid::DisplayMode::PARTICLE)) 
                        renderer::fluid::displayMode = renderer::fluid::DisplayMode::PARTICLE;
                    static float thicknessScaler = renderer::fluid::thicknessScaler;
                    ImGui::SliderFloat("Thickness Scaler", &thicknessScaler, 0.0f, 1.0f);
                    renderer::fluid::thicknessScaler = thicknessScaler;
                    ImGui::ColorEdit3("Fluid Color", renderer::fluid::fluidColor);
                    static float photonEnergy = renderer::sceneWithCaustics::uPhotonEnergy * 40.0f;

                    if (renderer::fluid::displayMode != renderer::fluid::DisplayMode::FOAM && renderer::fluid::displayMode != renderer::fluid::DisplayMode::CARTOON) {
                        ImGui::Checkbox("Enable Caustics", &renderer::enableCaustics);
                        if (renderer::enableCaustics) {
                            ImGui::SliderFloat("Photon Energy", &photonEnergy, 0.0f, 1.0f);
                            renderer::sceneWithCaustics::uPhotonEnergy = photonEnergy * 0.025f;
                            static float photonSize = renderer::sceneWithCaustics::uPhotonSize * 2.0f;
                            ImGui::SliderFloat("Photon Size", &photonSize, 0.0f, 1.0f);
                            renderer::sceneWithCaustics::uPhotonSize = photonSize * 0.5f;
                            ImGui::SliderInt("Blur Count", &renderer::sceneWithCaustics::blurCount, 0, 10);
                        }
                    }
                }

                // cartoon
                ImGui::Separator();

                if (renderer::fluid::displayMode == renderer::fluid::DisplayMode::CARTOON) {
                    ImGui::SliderFloat("Bright Threshold", &renderer::fluid::brightThreshold, 0.0f, 1.0f);
                    ImGui::SliderFloat("Dark Threshold", &renderer::fluid::darkThreshold, 0.0f, 1.0f);
                    if (renderer::fluid::darkThreshold > renderer::fluid::brightThreshold) {
                        renderer::fluid::darkThreshold = renderer::fluid::brightThreshold;
                    }
                    ImGui::SliderFloat("Bright Factor", &renderer::fluid::brightFactor, 1.0f, 5.0f);
                    ImGui::SliderFloat("Dark Factor", &renderer::fluid::darkFactor, 0.0f, 1.0f);

                    ImGui::SliderFloat("Refract Threshold", &renderer::fluid::refractThreshold, 0.0f, 1.0f);
                    ImGui::SliderFloat("Reflect Threshold", &renderer::fluid::reflectThreshold, 0.0f, 1.0f);
                    if (renderer::fluid::reflectThreshold > renderer::fluid::refractThreshold) {
                        renderer::fluid::reflectThreshold = renderer::fluid::refractThreshold;
                    }
                    ImGui::SliderFloat("Refract Max", &renderer::fluid::refractMax, 0.0f, 1.0f);
                    ImGui::SliderFloat("Reflect Max", &renderer::fluid::reflectMax, 0.0f, 1.0f);

                    ImGui::Separator();

                    ImGui::SliderInt("Edge Kernel Radius", &renderer::fluid::edgeKernelRadius, 1, 16);
                }
            }

            // simulation
            {
                {
                    ImGui::Separator();
                    ImGui::Checkbox("Simulation", &common::enableSimulation);
                    ImGui::SliderInt("Constraint Projection Iteration", &simulator::constraintProjectionIteration, 1, 32);
                    static float viscosity = simulator::viscosityParameter * 1e2f;
                    ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 1.0f);
                    simulator::viscosityParameter = viscosity * 1e-2f;
                    static float vorticity = simulator::vorticityParameter * 1e6f;
                    ImGui::SliderFloat("Vorticity", &vorticity, 0.0f, 1.0f);
                    simulator::vorticityParameter = vorticity * 1e-6f;
                    static float horizonMaxCoordinate = static_cast<float>(simulator::horizonMaxCoordinate / simulator::HORIZON_MAX_COORDINATE);
                    ImGui::SliderFloat("Horizon Max Coordinate", &horizonMaxCoordinate, 0.5f, 1.0f);
                    simulator::horizonMaxCoordinate = horizonMaxCoordinate * simulator::HORIZON_MAX_COORDINATE;
                    // ImGui::SliderInt("*Max Neighbor Count", &simulator::maxNeighborCount, 64, 256);
                }
            }

            // utils
            {
                ImGui::Separator();
                ImGui::Checkbox("Camera Mode", &common::cameraMode);
                ImGui::Text("[Esc] to exit camera model");

                ImGui::Separator();
                if (ImGui::Button("Reset")) 
                    common::resetSimulation = true;
                if (ImGui::Button("Exit"))
                    glfwSetWindowShouldClose(renderer::window::window, true);
                ImGui::End();
            }
        }

        // Performance Monitor
        if (showPerformanceMonitor) {
            ImGui::Begin("Performance Monitor");
            ImGui::Text("Particle Count: %d k", simulator::PARTICLE_COUNT / 1024);
            ImGui::Text("FPS: %.2f", common::fps);
            ImGui::Text("Frame Time: %.2f ms", common::totalTime);

            ImGui::Separator();
            ImGui::Text("Render Time: %.2f ms (%.2f %%)", common::renderTime, common::renderTime / common::totalTime * 100);
            ImGui::Text("Simulate Time: %.2f ms (%.2f %%)", common::simulateTime, common::simulateTime / common::totalTime * 100);
            ImGui::End();
        }

        // Render Performance
        if (showRenderPerformance) {
            ImGui::Begin("Render Performance");
            ImGui::Text("Render Total: %.2f ms", common::renderTime);

            ImGui::Separator();
            ImGui::Text("Process Input:          %.2f ms (%.2f%%)", common::renderTimeSlice[0], common::renderTimePercentage[0]);
            ImGui::Text("Background:             %.2f ms (%.2f%%)", common::renderTimeSlice[1], common::renderTimePercentage[1]);
            ImGui::Text("Copy Particle Attr:     %.2f ms (%.2f%%)", common::renderTimeSlice[2], common::renderTimePercentage[2]);
            ImGui::Text("Fluid Depth:           %.2f ms (%.2f%%)", common::renderTimeSlice[3], common::renderTimePercentage[3]);
            ImGui::Text("Smooth Fluid Depth:    %.2f ms (%.2f%%)", common::renderTimeSlice[4], common::renderTimePercentage[4]);
            ImGui::Text("Compute Fluid Normal:  %.2f ms (%.2f%%)", common::renderTimeSlice[5], common::renderTimePercentage[5]);
            ImGui::Text("Fluid Thickness:       %.2f ms (%.2f%%)", common::renderTimeSlice[6], common::renderTimePercentage[6]);
            ImGui::Text("Render Final Scene:    %.2f ms (%.2f%%)", common::renderTimeSlice[7], common::renderTimePercentage[7]);
            ImGui::Text("Swap Buffers:          %.2f ms (%.2f%%)", common::renderTimeSlice[8], common::renderTimePercentage[8]);
            ImGui::End();
        }

        // Simulate Performance
        if (showSimulatePerformance) {
            ImGui::Begin("Simulate Performance");
            ImGui::Text("Simulate Total: %.2f ms", common::simulateTime);

            ImGui::Separator();
            ImGui::Text("Apply External Force:      %.2f ms (%.2f%%)", common::simulateTimeSlice[0], common::simulateTimePercentage[0]);
            ImGui::Text("Search Neighbor:           %.2f ms (%.2f%%)", common::simulateTimeSlice[1], common::simulateTimePercentage[1]);
            ImGui::Text("Constraint Projection:     %.2f ms (%.2f%%)", common::simulateTimeSlice[2], common::simulateTimePercentage[2]);
            ImGui::Text("Update Velocity:           %.2f ms (%.2f%%)", common::simulateTimeSlice[3], common::simulateTimePercentage[3]);
            ImGui::Text("Vorticity Confinement:    %.2f ms (%.2f%%)", common::simulateTimeSlice[4], common::simulateTimePercentage[4]);
            ImGui::Text("Viscosity:                %.2f ms (%.2f%%)", common::simulateTimeSlice[5], common::simulateTimePercentage[5]);
            ImGui::Text("Handle Boundary Collision: %.2f ms (%.2f%%)", common::simulateTimeSlice[6], common::simulateTimePercentage[6]);
            ImGui::Text("Update Particle Position:  %.2f ms (%.2f%%)", common::simulateTimeSlice[7], common::simulateTimePercentage[7]);
            ImGui::End();
        }

        return 0;
    }
}
