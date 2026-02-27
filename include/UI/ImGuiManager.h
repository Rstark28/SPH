//
// Created by Robert Stark on 2/16/26.
//

#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

struct GLFWwindow;

/**
 * Singleton class to manage ImGui initialization and rendering.
 * It provides methods to start a new ImGui frame, render the UI, and configure UI elements.
 */
class ImGuiManager {
public:
    /**
     * Get the singleton instance of the ImGuiManager class.
     * @return Reference to the ImGuiManager instance.
     */
    static ImGuiManager& getInstance();

    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;
    ImGuiManager(ImGuiManager&&) = delete;
    ImGuiManager& operator=(ImGuiManager&&) = delete;

    /**
     * Initialize ImGui with the given GLFW window. This sets up the ImGui context and configures it
     * for use with OpenGL and GLFW.
     * @param window The GLFW window to associate with ImGui.
     */
    void init(GLFWwindow* window);

    /**
     * Begin a new ImGui frame. This should be called at the start of each frame before configuring
     * UI elements.
     */
    static void beginFrame();

    /**
     * Render the ImGui UI elements. This should be called after configuring the UI for the current
     * frame.
     */
    static void renderFrame();

    /**
     * Configure ImGui UI elements for the current frame. This is where you can create windows,
     * sliders, buttons, etc. to control simulation parameters.
     */
    static void configureUI();

private:
    ImGuiManager() = default;
    ~ImGuiManager();

    GLFWwindow* _window = nullptr;
    bool _initialized = false;
};

#endif // IMGUIMANAGER_H
