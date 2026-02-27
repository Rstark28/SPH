//
// Created by Robert Stark on 2/16/26.
//

#include "../../include/UI/ImGuiManager.h"
#include "Math/SPH.h"
#include "Rules.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

ImGuiManager& ImGuiManager::getInstance()
{
    static ImGuiManager instance;
    return instance;
}

void ImGuiManager::init(GLFWwindow* window)
{
    if (_initialized)
        return;

    _window = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    _initialized = true;
}

ImGuiManager::~ImGuiManager()
{
    if (_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void ImGuiManager::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::configureUI()
{
    SPH& sph = SPH::getInstance();
    SPHConfig config = sph.config();
    bool configChanged = false;

    if (!ImGui::Begin("SPH Config")) {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("SPH Config");
    configChanged |= ImGui::SliderFloat("Gravity", &config.gravity, -50.0f, 0.0f, "%.2f");
    configChanged
        |= ImGui::SliderFloat("Target Density", &config.targetDensity, 10.0f, 2000.0f, "%.1f");
    configChanged
        |= ImGui::SliderFloat("Pressure Mult", &config.pressureMultiplier, 0.0f, 2000.0f, "%.1f");
    configChanged |= ImGui::SliderFloat(
        "Near Pressure Mult", &config.nearPressureMultiplier, 0.0f, 50.0f, "%.2f");
    configChanged
        |= ImGui::SliderFloat("Viscosity Strength", &config.viscosityStrength, 0.0f, 5.0f, "%.3f");
    configChanged
        |= ImGui::SliderFloat("Collision Damping", &config.collisionDamping, 0.0f, 1.0f, "%.2f");
    configChanged |= ImGui::SliderFloat3("Bounds", &config.bounds[0], 0.1f, 5.0f, "%.2f");

    if (configChanged) {
        sph.setConfig(config);
    }

    ImGui::Separator();
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::Text(
        "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void ImGuiManager::renderFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
