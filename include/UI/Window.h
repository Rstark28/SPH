#ifndef WINDOW_H
#define WINDOW_H

struct GLFWwindow;

/**
 * Singleton class to manage the GLFW window and input callbacks.
 */
class Window {
public:
    /**
     * Get the singleton instance of the Window class.
     * @return Reference to the Window instance.
     */
    static Window& getInstance();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    /**
     * Initialize the GLFW window with the specified width, height, and title.
     * @param width The width of the window in pixels.
     * @param height The height of the window in pixels.
     * @param title The title of the window.
     */
    void init(int width, int height, const char* title);

    /**
     * Check if the window should close (e.g., if the user has requested to close it).
     * @return True if the window should close, false otherwise.
     */
    [[nodiscard]] bool shouldClose() const;

    /**
     * Poll for and process events (e.g., keyboard and mouse input).
     */
    static void pollEvents();

    /**
     * Swap the front and back buffers, displaying the rendered frame.
     */
    void swapBuffers() const;

    /**
     * Begin a new ImGui frame, preparing for UI rendering.
     */
    static void beginImGuiFrame();

    /**
     * Render the ImGui UI elements.
     */
    static void renderImGui();

    /**
     * Configure ImGui UI elements for the current frame (e.g., create windows, sliders, etc.).
     */
    static void configureImGui();

    /**
     * Get the underlying GLFW window pointer.
     * @return Pointer to the GLFWwindow.
     */
    [[nodiscard]] GLFWwindow* getWindow() const
    {
        return _window;
    }

private:
    friend class Renderer;
    friend class Camera;

    Window() = default;
    ~Window();

    GLFWwindow* _window = nullptr;
    int _height = 0;
    int _width = 0;
};

#endif // WINDOW_H
