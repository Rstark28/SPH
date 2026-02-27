#ifndef SPH_H
#define SPH_H

#include "Particle.h"

#include <barrier>
#include <thread>
#include <vector>

struct SPHConfig {
    float gravity = -9.81f;
    float smoothingRadius = 0.2f;
    float targetDensity = 1000.0f;
    float pressureMultiplier = 30.0f;
    float nearPressureMultiplier = 25.0f;
    float viscosityStrength = 0.035f;
    float collisionDamping = 0.85f;
    Vec3<float> bounds { 1.0f, 1.0f, 1.0f };
};

/*
 * SPH (Smoothed Particle Hydrodynamics) class that implements the core simulation logic for fluid
 * dynamics. This CPU implementation steps particles through: 1) External forces + prediction 2)
 * Neighbor search (spatial hashing) 3) Density/pressure computation 4) Pressure + viscosity forces
 * 5) Position update + boundary collisions
 */
class SPH {
public:
    /** Get the singleton instance of the SPH simulation.
     */
    static SPH& getInstance();

    /** Initialize the SPH simulation with the given configuration and initial particles.
     * @param config The simulation parameters (gravity, smoothing radius, etc.).
     * @param particles The initial set of particles to simulate.
     */
    void init(SPHConfig config = {}, const std::vector<Particle>& particles = {});

    /** Destructor for the SPH simulation, responsible for cleaning up resources and stopping worker
     * threads.
     */
    ~SPH();

    /** Step the simulation forward by a given time delta.
     * @param dt The time step to advance the simulation.
     */
    void step();

    /** Update the simulation configuration parameters.
     * @param config The new configuration to apply to the simulation.
     */
    void setConfig(const SPHConfig& config);

    /** Get the current simulation configuration.
     * @return A const reference to the current SPHConfig.
     */
    [[nodiscard]] const SPHConfig& config() const;

    /** Get the current list of particles in the simulation.
     * @return A const reference to the vector of particles.
     */
    [[nodiscard]] const std::vector<Particle>& particles() const
    {
        return _particles;
    }

    /** Get a non-const reference to the current list of particles (for modification).
     * @return A reference to the vector of particles.
     */
    [[nodiscard]] std::vector<Particle>& particles()
    {
        return _particles;
    }

private:
    SPHConfig _config;
    float _dt = 1 / 60.0f;
    std::vector<Particle> _particles;

    // Multithreading members.
    std::vector<std::thread> _threads;
    std::unique_ptr<std::barrier<>> _barrier;
    std::atomic<bool> _running { true };
    size_t _chunk;

    // Precomputed kernel constants (depend on smoothingRadius).
    float K_SpikyPow2 = 0.0f;
    float K_SpikyPow3 = 0.0f;
    float K_SpikyPow2Grad = 0.0f;
    float K_SpikyPow3Grad = 0.0f;

    explicit SPH() = default;

    /**
     * Main function executed by each worker thread to perform a simulation step. This function will
     * synchronize with other threads using a barrier to ensure all threads are at the same point in
     * the simulation before proceeding to the next step.
     * @param thread The index of the thread to determine which portion of the particle list to
     * process.
     */
    void threadStep(size_t thread);

    /**
     * Keeps the worker thread running in a loop, continuously performing simulation steps until the
     * simulation is stopped.
     * @param thread The index of the thread to determine which portion of the particle list to
     * process.
     */
    void threadLoop(size_t thread);

    // Kernel functions used for density/pressure/viscosity.
    [[nodiscard]] float densityKernel(float dst) const;
    [[nodiscard]] float nearDensityKernel(float dst) const;
    [[nodiscard]] float densityDerivative(float dst) const;
    [[nodiscard]] float nearDensityDerivative(float dst) const;
    [[nodiscard]] float poly6Kernel(float dst) const;
    [[nodiscard]] float pressureFromDensity(float density) const;
    [[nodiscard]] float nearPressureFromDensity(float nearDensity) const;

    // Spatial hashing for neighbor lookup.
    static const Vec3<int> OFFSETS_3D[27];
    [[nodiscard]] Vec3<int> getCell(const Particle& particle) const;
    static int hash(const Vec3<int>& cell);
    [[nodiscard]] uint32_t keyFromHash(uint32_t hash) const;

    /** Resolve collisions with the simulation bounds and apply damping.
     * @param particle The particle to check for collisions and resolve.
     */
    void resolveCollisions(Particle& particle) const;

    /** Apply gravity to the particles and predict their new positions.
     * @param start An iterator pointing to the start of the particle range to process.
     * @param end An iterator pointing to the end of the particle range to process.
     */
    void applyExternalForces(auto start, auto end);

    /** Build the spatial hash for efficient neighbor searching. This involves:
     */
    void buildSpatialHash();

    /** Reorder the particles in memory based on the sorted keys to improve cache locality during
     * neighbor searches. This should be called after building the spatial hash to ensure particles
     * are stored in an order that reflects their spatial locality.
     */
    void reorderParticles();

    /** Calculate the density and near-density for each particle based on its neighbors.
     * @param start An iterator pointing to the start of the particle range to process.
     * @param end An iterator pointing to the end of the particle range to process.
     */
    void calculateDensities(auto start, auto end);

    /** Calculate the pressure force for each particle based on its density and the densities of its
     * neighbors.
     * @param start An iterator pointing to the start of the particle range to process.
     * @param end An iterator pointing to the end of the particle range to process.
     */
    void calculatePressureForce(auto start, auto end);

    /** Calculate the viscosity force for each particle based on the velocities of its neighbors.
     * @param velocitySnapshot A snapshot of the particle velocities to use for calculating
     * viscosity forces.
     * @param start An iterator pointing to the start of the particle range to process.
     * @param end An iterator pointing to the end of the particle range to process.
     */
    void calculateViscosity(auto start, auto end);

    /** Update the positions of the particles based on their velocities and resolve any collisions
     * with the bounds.
     * @param start An iterator pointing to the start of the particle range to process.
     * @param end An iterator pointing to the end of the particle range to process.
     */
    void updatePositions(auto start, auto end);

    // Working buffers to avoid reallocating every step.
    std::vector<uint32_t> _keys;
    std::vector<uint32_t> _sortedIndices;
    std::vector<uint32_t> _offsets;
    std::vector<Particle> _reorderBuffer;
    std::vector<Vec3<float>> _velocitySnapshot;
    bool _useViscosity = true;
};

#endif // SPH_H
