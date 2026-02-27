//
// Created by Robert Stark on 2/15/26.
//

#ifndef PARTICLE_H
#define PARTICLE_H
#include <Math/Vec.h>
#include <UI/Mesh.h>
#include <cstdint>

/**
 * Represents a single particle in the SPH simulation.
 * Contains position, velocity, density, and other properties.
 * Provides methods for drawing the particle using OpenGL.
 */
class Particle {
public:
    /**
     * Initializes the static mesh used for rendering particles.
     * Should be called once before creating any Particle instances.
     *
     * @param shader The OpenGL shader program to use for rendering the particles.
     */
    static void init(uint32_t shader);

    Particle() = default;

    /**
     * Constructs a Particle with the given position and velocity.
     *
     * @param position The initial position of the particle in 3D space.
     * @param velocity The initial velocity of the particle in 3D space.
     */
    Particle(Vec3<float> position, Vec3<float> velocity);

    /**
     * Draws the particle using the provided shader program.
     */
    void draw() const;

    /**
     * Checks if this particle is the same as another particle (i.e., they are the same instance).
     */
    bool operator==(const Particle& other) const
    {
        return this == &other;
    }

    Vec3<float> _position {};
    Vec3<float> _predicted {}; // Predicted position for the current time step
    Vec3<float> _velocity {};
    float _density = 0.0f; // Density based on the smoothing kernel
    float _nearDensity = 0.0f; // Near density for pressure calculations

private:
    static float _radius;
    static uint32_t _shader;
    static Mesh _mesh;
};

#endif // PARTICLE_H
