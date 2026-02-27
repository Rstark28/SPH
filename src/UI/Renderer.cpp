#include "../../include/UI/Renderer.h"
#include "../../include/Math/SPH.h"
#include "../../include/UI/Camera.h"
#include "../../include/UI/Mesh.h"
#include "Rules.h"
#include "UI/Window.h"
#include <Particle.h>
#include <cmath>
#include <glad/glad.h>

Renderer& Renderer::getInstance()
{
    static Renderer instance;
    return instance;
}

bool Renderer::init()
{
    const auto vertexSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform vec3 uOffset;
        uniform mat4 uProjection;
        uniform mat4 uView;

        void main()
        {
            gl_Position = uProjection * uView * vec4(aPos + uOffset, 1.0);
        }
    )";

    const auto fragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;

        uniform vec3 uColor;

        void main()
        {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    const uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, nullptr);
    glCompileShader(vs);

    const uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, nullptr);
    glCompileShader(fs);

    _shaderProgram = glCreateProgram();
    glAttachShader(_shaderProgram, vs);
    glAttachShader(_shaderProgram, fs);
    glLinkProgram(_shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    glEnable(GL_DEPTH_TEST);

    Particle::init(_shaderProgram);

    const auto boxVertexSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 uProjection;
        uniform mat4 uView;

        void main()
        {
            gl_Position = uProjection * uView * vec4(aPos, 1.0);
        }
    )";

    const auto boxFragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(0.3, 0.7, 1.0, 1.0); // Light blue for the box
        }
    )";

    const uint32_t boxVs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(boxVs, 1, &boxVertexSrc, nullptr);
    glCompileShader(boxVs);

    const uint32_t boxFs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(boxFs, 1, &boxFragmentSrc, nullptr);
    glCompileShader(boxFs);

    _boxShaderProgram = glCreateProgram();
    glAttachShader(_boxShaderProgram, boxVs);
    glAttachShader(_boxShaderProgram, boxFs);
    glLinkProgram(_boxShaderProgram);

    glDeleteShader(boxVs);
    glDeleteShader(boxFs);

    _boxMesh = MeshFactory::createBox(SPH::getInstance().config().bounds);

    _camera = &Camera::getInstance();
    const Window& window = Window::getInstance();
    _aspect = static_cast<float>(window._width) / static_cast<float>(window._height);

    // Initialize SPH with particles
    SPH& sph = SPH::getInstance();
    const auto initialParticles = spawnParticlesInBox(10000, 2.0f, 0.05f, 0.5f);
    sph.init({}, initialParticles);

    return true;
}

void Renderer::draw()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shaderProgram);

    // Update view for Camera
    const auto projection = _camera->getProjectionMatrix(_aspect);
    const auto view = _camera->getViewMatrix();
    const int projLoc = glGetUniformLocation(_shaderProgram, "uProjection");
    const int viewLoc = glGetUniformLocation(_shaderProgram, "uView");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

    // Draw particles
    SPH& sph = SPH::getInstance();
    sph.step();
    const auto& particles = sph.particles();
    for (const auto& particle : particles) {
        particle.draw();
    }

    // Refresh box mesh if bounds changed
    const SPHConfig& config = sph.config();
    if (const auto diff = config.bounds - _boxHalfSize;
        std::abs(diff[0]) > 1e-4f || std::abs(diff[1]) > 1e-4f || std::abs(diff[2]) > 1e-4f) {
        _boxMesh = MeshFactory::createBox(config.bounds);
    }

    // Draw the box wireframe
    glUseProgram(_boxShaderProgram);
    glUniformMatrix4fv(
        glGetUniformLocation(_boxShaderProgram, "uProjection"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(_boxShaderProgram, "uView"), 1, GL_FALSE, view);
    _boxMesh.draw();
}
