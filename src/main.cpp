#include "../include/UI/Camera.h"
#include "../include/UI/Renderer.h"
#include "../include/UI/Window.h"
#include <GLFW/glfw3.h>

int main()
{
    Window& window = Window::getInstance();
    window.init(900, 900, "Particle Simulator");

    Renderer& renderer = Renderer::getInstance();
    if (!renderer.init()) {
        return -1;
    }

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!window.shouldClose()) {
        const auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Camera::getInstance().processInput(window.getWindow(), deltaTime);

        Window::pollEvents();

        Window::beginImGuiFrame();
        Window::configureImGui();

        renderer.draw();

        Window::renderImGui();
        window.swapBuffers();
    }

    return 0;
}
