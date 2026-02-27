//
// Created by Robert Stark on 2/16/26.
//

#include "Particle.h"
#include "../include/UI/Renderer.h"
#include "glad/glad.h"
#include <algorithm>

float Particle::_radius = 0.02f;
Mesh Particle::_mesh;
uint32_t Particle::_shader;

Particle::Particle(const Vec3<float> position, const Vec3<float> velocity)
    : _position(position)
    , _predicted(position)
    , _velocity(velocity)
{
}

void Particle::init(uint32_t shader)
{
    _mesh = MeshFactory::createSphere(_radius);
    _shader = shader;
}

void Particle::draw() const
{
    glUniform3f(glGetUniformLocation(_shader, "uOffset"), _position[0], _position[1], _position[2]);

    const float r = std::clamp(_velocity.norm() / 5.0f, 0.0f, 1.0f);
    const float g = 0.2f + (1.0f - r) * 0.3f;
    const float b = 1.0f - r;
    glUniform3f(glGetUniformLocation(_shader, "uColor"), r, g, b);

    _mesh.draw();
}
