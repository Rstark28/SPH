#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "Math/Vec.h"
#include "Mesh.h"

/**
 * Singleton class responsible for rendering the particles and the box.
 * It manages shader programs and mesh data for rendering.
 */
class Renderer {
public:
    /**
     * Get the singleton instance of the Renderer class.
     * @return Reference to the Renderer instance.
     */
    static Renderer& getInstance();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /**
     * Initialize the renderer by compiling shaders and setting up mesh data.
     * @return True if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * Render the current frame.
     */
    void draw();

private:
    friend class Window;
    friend class Camera;

    Renderer() = default;
    ~Renderer() = default;

    uint32_t _shaderProgram = 0;
    uint32_t _boxShaderProgram = 0;
    Mesh _boxMesh;
    Vec3<float> _boxHalfSize { 1.0f, 1.0f, 1.0f };

    Camera* _camera {};
    float _aspect = 1.0f;
};

#endif // RENDERER_H
