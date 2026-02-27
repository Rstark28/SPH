#include "../../include/UI/Window.h"
#include "../../include/UI/Camera.h"
#include "../../include/UI/ImGuiManager.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

Window& Window::getInstance()
{
    static Window instance;
    return instance;
}

void Window::init(const int width, const int height, const char* title)
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    _height = height;
    _width = width;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _window = glfwCreateWindow(_width, _height, title, nullptr, nullptr);
    if (!_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(_window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    int w, h;
    glfwGetFramebufferSize(_window, &w, &h);
    glViewport(0, 0, w, h);

    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    ImGuiManager::getInstance().init(_window);
    Camera::getInstance();
}

Window::~Window()
{
    if (_window) {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return _window ? glfwWindowShouldClose(_window) : true;
}

void Window::swapBuffers() const
{
    if (_window)
        glfwSwapBuffers(_window);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::beginImGuiFrame()
{
    ImGuiManager::beginFrame();
}

void Window::renderImGui()
{
    ImGuiManager::renderFrame();
}

void Window::configureImGui()
{
    ImGuiManager::configureUI();
}
