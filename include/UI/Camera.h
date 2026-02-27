//
// Created by Robert Stark on 2/16/26.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "../Math/Vec.h"
#include <array>

struct GLFWwindow;

/**
 * Camera class to manage the view and projection matrices for rendering the 3D scene.
 * It handles user input for moving, rotating, and zooming the camera via keyboard input only.
 *
 * Controls:
 * W: Move forward
 * S: Move backward
 * A: Move left
 * D: Move right
 * Space: Move up
 * Left Shift: Move down
 * Q: Rotate left (yaw)
 * E: Rotate right (yaw)
 * Up Arrow: Rotate up (pitch)
 * Down Arrow: Rotate down (pitch)
 * +/=: Zoom in (increase FOV)
 * -: Zoom out (decrease FOV)
 */
class Camera {
public:
    /**
     * Get the singleton instance of the Camera class.
     * @return Reference to the Camera instance.
     */
    static Camera& getInstance();

    // Delete copy/move operations
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    /**
     * Process keyboard input for camera movement, rotation, and zoom.
     * Movement: W/S/A/D for forward/backward/left/right, Space/Shift for up/down
     * Rotation: Q/E for yaw (left/right), Up/Down arrows for pitch (up/down)
     * Zoom: +/- or Equals/Minus keys for zooming in/out
     * @param window The GLFW window to check for key presses.
     * @param deltaTime The time elapsed since the last frame, used to ensure consistent movement
     * speed.
     */
    void processInput(GLFWwindow* window, float deltaTime);

    /**
     * Update the view matrix based on the current camera position and orientation.
     */
    const float* getViewMatrix();

    /**
     * Update the projection matrix based on the current field of view and aspect ratio.
     * @param aspect The aspect ratio of the viewport (width/height).
     */
    const float* getProjectionMatrix(float aspect);

private:
    friend class Window;
    friend class Renderer;

    Camera();
    ~Camera() = default;

    /**
     * Recalculate the camera's front, right, and up vectors based on the current yaw and pitch
     * angles. This should be called whenever the camera's orientation changes.
     */
    void updateCameraVectors();

    // Camera vectors
    Vec3<float> _position { 0.0f, 1.5f, 3.5f };
    Vec3<float> _front { 0.0f, 0.0f, -1.0f };
    Vec3<float> _up { 0.0f, 1.0f, 0.0f };
    Vec3<float> _right { 1.0f, 0.0f, 0.0f };
    const Vec3<float> _worldUp { 0.0f, 1.0f, 0.0f };

    // Euler angles (degrees0
    float _yaw = -90.0f;
    float _pitch = -22.5f;
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;

    // Camera settings
    const float _movementSpeed = 2.5f; // units per second
    const float _rotationSpeed = 45.0f; // degrees per second
    const float _zoomSpeed = 30.0f; // degrees per second
    float _fov = 65.0f; // degrees
    static constexpr float MIN_ZOOM = 1.0f;
    static constexpr float MAX_ZOOM = 90.0f;
    const float _nearZoom = 1.0f;
    const float _farZoom = 30.0f;

    // Cached matrices
    float _viewMatrix[16] {};
    float _projectionMatrix[16] {};
};

#endif // CAMERA_H
