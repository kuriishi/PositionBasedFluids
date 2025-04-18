#include "gui.hpp"

#include "../../include/imgui/imgui.h"
#include "../../include/imgui/imgui_impl_glfw.h"
#include "../../include/imgui/imgui_impl_opengl3.h"

#include "../renderer/window.hpp"
#include "../common/performance_log.hpp"
#include "../renderer/renderer.hpp"
#include "../simulator/simulator.hpp"

namespace gui {
    int guiInit() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.FontGlobalScale = 2.0f;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(renderer::window, true);
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
        glfwGetFramebufferSize(renderer::window, &display_w, &display_h);

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
                if (ImGui::RadioButton("Fluid", renderer::displayMode == renderer::DisplayMode::FLUID)) 
                    renderer::displayMode = renderer::DisplayMode::FLUID;
                if (ImGui::RadioButton("Thickness", renderer::displayMode == renderer::DisplayMode::THICKNESS)) 
                    renderer::displayMode = renderer::DisplayMode::THICKNESS;
                if (ImGui::RadioButton("Normal", renderer::displayMode == renderer::DisplayMode::NORMAL)) 
                    renderer::displayMode = renderer::DisplayMode::NORMAL;
                if (ImGui::RadioButton("Depth", renderer::displayMode == renderer::DisplayMode::DEPTH)) 
                    renderer::displayMode = renderer::DisplayMode::DEPTH;
                if (ImGui::RadioButton("Particle", renderer::displayMode == renderer::DisplayMode::PARTICLE)) 
                    renderer::displayMode = renderer::DisplayMode::PARTICLE;
                ImGui::Checkbox("Smooth Depth", &renderer::enableSmoothDepth);
                ImGui::SliderInt("Smooth Iteration", &renderer::smoothIteration, 1, 32);
                static float particleRadiusScaler = renderer::particleRadiusScaler * 0.1f;
                ImGui::SliderFloat("Particle Radius Scaler", &particleRadiusScaler, 0.0f, 1.0f);
                renderer::particleRadiusScaler = particleRadiusScaler * 10.0f;
                static float minimumDensityScaler = renderer::minimumDensityScaler * 0.5f;
                ImGui::SliderFloat("Minimum Density Scaler", &minimumDensityScaler, 0.0f, 1.0f);
                renderer::minimumDensityScaler = minimumDensityScaler * 2.0f;
                static float thicknessScaler = renderer::thicknessScaler;
                ImGui::SliderFloat("Thickness Scaler", &thicknessScaler, 0.0f, 1.0f);
                renderer::thicknessScaler = thicknessScaler;
                ImGui::ColorEdit3("Fluid Color", renderer::fluidColor);
            }

            // simulation
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

            // utils
            {
                ImGui::Separator();
                ImGui::Checkbox("Camera Movement", &common::enableCameraMovement);
                ImGui::Text("[Esc] to exit camera model");

                ImGui::Separator();
                if (ImGui::Button("Reset")) 
                    common::resetSimulation = true;
                if (ImGui::Button("Exit"))
                    glfwSetWindowShouldClose(renderer::window, true);
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
