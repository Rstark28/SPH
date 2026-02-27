//
// Created by Robert Stark on 2/16/26.
//

#include "../../include/UI/Camera.h"
#include "Rules.h"
#include "imgui_internal.h"
#include <GLFW/glfw3.h>

Camera& Camera::getInstance()
{
    static Camera instance;
    return instance;
}

Camera::Camera()
{
    updateCameraVectors();
}

void Camera::processInput(GLFWwindow* window, const float deltaTime)
{
    const float velocity = _movementSpeed * deltaTime;
    const float rotationVelocity = _rotationSpeed * deltaTime;
    const float zoomVelocity = _zoomSpeed * deltaTime;

    // Movement controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        _position += _front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        _position -= _front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        _position -= _right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        _position += _right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        _position[1] += velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        _position[1] -= velocity;
    }

    // Rotation controls (Q/E for yaw, up/down arrows for pitch)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        _yaw -= rotationVelocity;
        updateCameraVectors();
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        _yaw += rotationVelocity;
        updateCameraVectors();
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        _pitch += rotationVelocity;
        _pitch = std::clamp(_pitch, MIN_PITCH, MAX_PITCH);
        updateCameraVectors();
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        _pitch -= rotationVelocity;
        _pitch = std::clamp(_pitch, MIN_PITCH, MAX_PITCH);
        updateCameraVectors();
    }

    // Zoom controls (+/= to zoom in, - to zoom out)
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        _fov -= zoomVelocity;
        _fov = std::clamp(_fov, MIN_ZOOM, MAX_ZOOM);
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        _fov += zoomVelocity;
        _fov = std::clamp(_fov, MIN_ZOOM, MAX_ZOOM);
    }
}

const float* Camera::getViewMatrix()
{
    _viewMatrix[0] = _right[0];
    _viewMatrix[1] = _up[0];
    _viewMatrix[2] = -_front[0];
    _viewMatrix[3] = 0.0f;
    _viewMatrix[4] = _right[1];
    _viewMatrix[5] = _up[1];
    _viewMatrix[6] = -_front[1];
    _viewMatrix[7] = 0.0f;
    _viewMatrix[8] = _right[2];
    _viewMatrix[9] = _up[2];
    _viewMatrix[10] = -_front[2];
    _viewMatrix[11] = 0.0f;
    _viewMatrix[12] = -(_right * _position);
    _viewMatrix[13] = -(_up * _position);
    _viewMatrix[14] = (_front * _position);
    _viewMatrix[15] = 1.0f;

    return _viewMatrix;
}

const float* Camera::getProjectionMatrix(const float aspect)
{
    memset(_projectionMatrix, 0, sizeof(_projectionMatrix));
    const float tanHalfFov = std::tan(toRadians(_fov) / 2.0f);

    _projectionMatrix[0] = 1.0f / (aspect * tanHalfFov);
    _projectionMatrix[5] = 1.0f / tanHalfFov;
    _projectionMatrix[10] = -(_farZoom + _nearZoom) / (_farZoom - _nearZoom);
    _projectionMatrix[11] = -1.0f;
    _projectionMatrix[14] = -(2.0f * _farZoom * _nearZoom) / (_farZoom - _nearZoom);
    return _projectionMatrix;
}

void Camera::updateCameraVectors()
{
    const float yawRad = toRadians(_yaw);
    const float pitchRad = toRadians(_pitch);

    _front = { std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad) };
    _front.normalize();
    _right = _front | _worldUp;
    _right.normalize();
    _up = _right | _front;
    _up.normalize();
}
