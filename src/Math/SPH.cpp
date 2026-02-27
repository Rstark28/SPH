#include "Math/SPH.h"
#include <algorithm>
#include <barrier>
#include <cmath>
#include <numeric>
#include <objc/objc.h>
#include <ranges>
#include <thread>
#include <utility>

#include "Rules.h"

SPH& SPH::getInstance()
{
    static SPH instance {};
    return instance;
}

void SPH::init(SPHConfig config, const std::vector<Particle>& particles)
{
    _config = std::move(config);
    _particles = particles;

    const size_t n = _particles.size();
    _keys.resize(n);
    _sortedIndices.resize(n);
    _offsets.resize(n);
    _reorderBuffer.resize(n);
    _velocitySnapshot.resize(n);

    const uint32_t threadCount = std::min<uint32_t>(
        std::max(1u, std::thread::hardware_concurrency()), static_cast<uint32_t>(n));
    _threads.resize(threadCount);

    const float smoothingRadius = _config.smoothingRadius;
    K_SpikyPow2 = 15.0f / (2.0f * PI * std::pow(smoothingRadius, 5));
    K_SpikyPow3 = 15.0f / (PI * std::pow(smoothingRadius, 6));
    K_SpikyPow2Grad = 15.0f / (PI * std::pow(smoothingRadius, 5));
    K_SpikyPow3Grad = 45.0f / (PI * std::pow(smoothingRadius, 6));
}

// Offsets for the 3x3x3 neighborhood around a cell (including the cell itself).
const Vec3<int> SPH::OFFSETS_3D[27] = { { -1, -1, -1 }, { 0, -1, -1 }, { 1, -1, -1 }, { -1, 0, -1 },
    { 0, 0, -1 }, { 1, 0, -1 }, { -1, 1, -1 }, { 0, 1, -1 }, { 1, 1, -1 }, { -1, -1, 0 },
    { 0, -1, 0 }, { 1, -1, 0 }, { -1, 0, 0 }, { 0, 0, 0 }, { 1, 0, 0 }, { -1, 1, 0 }, { 0, 1, 0 },
    { 1, 1, 0 }, { -1, -1, 1 }, { 0, -1, 1 }, { 1, -1, 1 }, { -1, 0, 1 }, { 0, 0, 1 }, { 1, 0, 1 },
    { -1, 1, 1 }, { 0, 1, 1 }, { 1, 1, 1 } };

// Fast integer hash constants for cell coordinates.
static constexpr int HASH_X = 73856093;
static constexpr int HASH_Y = 19349663;
static constexpr int HASH_Z = 83492791;

void SPH::setConfig(const SPHConfig& config)
{
    _config = config;
}

const SPHConfig& SPH::config() const
{
    return _config;
}

float SPH::densityKernel(const float distance) const
{
    if (const float h = _config.smoothingRadius; distance < h) {
        const float v = h - distance;
        return v * v * K_SpikyPow2;
    }
    return 0;
}

float SPH::nearDensityKernel(const float distance) const
{
    if (const float h = _config.smoothingRadius; distance < h) {
        const float v = h - distance;
        return v * v * v * K_SpikyPow3;
    }
    return 0;
}

float SPH::densityDerivative(const float distance) const
{
    if (const float h = _config.smoothingRadius; distance <= h) {
        const float v = h - distance;
        return -v * K_SpikyPow2Grad;
    }
    return 0;
}

float SPH::nearDensityDerivative(const float distance) const
{
    if (const float h = _config.smoothingRadius; distance <= h) {
        const float v = h - distance;
        return -v * v * K_SpikyPow3Grad;
    }
    return 0;
}

float SPH::poly6Kernel(const float distance) const
{
    if (const float h = _config.smoothingRadius; distance < h) {
        const float scale = 315.0f / (64.0f * PI * std::pow(h, 9));
        const float v = h * h - distance * distance;
        return v * v * v * scale;
    }
    return 0;
}

float SPH::pressureFromDensity(const float density) const
{
    return (density - _config.targetDensity) * _config.pressureMultiplier;
}

float SPH::nearPressureFromDensity(const float nearDensity) const
{
    return nearDensity * _config.nearPressureMultiplier;
}

Vec3<int> SPH::getCell(const Particle& particle) const
{
    return Vec3<int>(particle._predicted / _config.smoothingRadius);
}

int SPH::hash(const Vec3<int>& cell)
{
    return cell[0] * HASH_X ^ cell[1] * HASH_Y ^ cell[2] * HASH_Z;
}

uint32_t SPH::keyFromHash(const uint32_t hash) const
{
    return hash % _particles.size();
}

void SPH::resolveCollisions(Particle& particle) const
{
    auto sign = [](const float v) { return v >= 0 ? 1.0f : -1.0f; };
    for (auto&& [position, velocity, halfBound] :
        std::views::zip(particle._position, particle._velocity, _config.bounds)) {
        if (halfBound - std::abs(position) <= 0) {
            position = halfBound * sign(position);
            velocity *= -_config.collisionDamping;
        }
    }
}

void SPH::step(float dt)
{
    const size_t threadCount = _threads.size();
    const size_t chunk = (_particles.size() + threadCount - 1) / threadCount;
    _threads.clear();

    // 1) Apply external forces and compute predicted positions.
    for (size_t t = 0; t < threadCount; ++t) {
        auto start = _particles.begin() + t * chunk;
        auto end = std::min(start + chunk, _particles.end());
        _threads.emplace_back([this, dt, start, end]() { applyExternalForces(dt, start, end); });
    }
    for (auto& thread : _threads) {
        thread.join();
    }

    // 2) Build spatial hash and reorder particles by cell key for cache-friendly neighbor lookups.
    buildSpatialHash();
    reorderParticles();

    // 3) Compute the starting index for each cell key in the sorted list.
    std::ranges::fill(_offsets, static_cast<uint32_t>(_particles.size()));
    for (uint32_t i = 0; i < _offsets.size(); ++i) {
        const uint32_t k = _keys[i];
        if (_offsets[k] > i)
            _offsets[k] = i;
    }

    _useViscosity = _config.viscosityStrength != 0.0f;

    // 4) Density -> pressure -> viscosity -> position update (synchronized across threads).
    std::barrier rendezvous(threadCount);
    _threads.clear();

    for (uint32_t t = 0; t < threadCount; ++t) {
        const auto start = _particles.begin() + t * chunk;
        const auto end = std::min(start + chunk, _particles.end());
        _threads.emplace_back([this, &rendezvous, start, end, dt]() {
            calculateDensities(start, end);
            rendezvous.arrive_and_wait();

            calculatePressureForce(dt, start, end);

            if (_useViscosity) {
                rendezvous.arrive_and_wait();

                for (auto particleIt = start; particleIt != end; ++particleIt) {
                    uint32_t i = particleIt - _particles.begin();
                    _velocitySnapshot[i] = _particles[i]._velocity;
                }

                rendezvous.arrive_and_wait();
                calculateViscosity(_velocitySnapshot, dt, start, end);
            }

            rendezvous.arrive_and_wait();

            updatePositions(dt, start, end);
            rendezvous.arrive_and_wait();
        });
    }

    for (auto& thread : _threads) {
        thread.join();
    }
}

void SPH::applyExternalForces(float dt, const auto start, const auto end)
{
    for (auto particleIt = start; particleIt != end; ++particleIt) {
        auto& particle = *particleIt;
        particle._velocity[1] += _config.gravity * dt;
        particle._predicted = particle._position + particle._velocity * dt;
    }
}

void SPH::buildSpatialHash()
{
    for (auto&& [particle, key] : std::views::zip(_particles, _keys)) {
        key = keyFromHash(hash(getCell(particle)));
    }

    std::iota(_sortedIndices.begin(), _sortedIndices.end(), 0);
    std::ranges::sort(
        _sortedIndices, [this](const uint32_t a, const uint32_t b) { return _keys[a] < _keys[b]; });
}

void SPH::reorderParticles()
{
    const auto keysCopy(_keys);
    for (auto&& [sortedIndex, key, buffer] :
        std::views::zip(_sortedIndices, _keys, _reorderBuffer)) {
        buffer = _particles[sortedIndex];
        key = keysCopy[sortedIndex];
    }
    _particles = _reorderBuffer;
}

void SPH::calculateDensities(const auto start, const auto end)
{
    const float squareRadius = std::pow(_config.smoothingRadius, 2.0f);

    for (auto particleIt = start; particleIt != end; ++particleIt) {
        auto& particle = *particleIt;
        const auto originCell = getCell(particle);
        float density = 0.0f;
        float nearDensity = 0.0f;

        for (const auto offset : OFFSETS_3D) {
            const auto cell = originCell + offset;
            const uint32_t key = keyFromHash(hash(cell));
            uint32_t neighborIndex = _offsets[key];

            while (neighborIndex < _particles.size() && _keys[neighborIndex] == key) {
                const auto distanceToNeighbor
                    = _particles[neighborIndex]._predicted - particle._predicted;
                const float squareDistance = distanceToNeighbor * distanceToNeighbor;
                ++neighborIndex;
                if (squareDistance <= squareRadius) {
                    const float distance = distanceToNeighbor.norm();
                    density += densityKernel(distance);
                    nearDensity += nearDensityKernel(distance);
                }
            }
        }

        particle._density = density;
        particle._nearDensity = nearDensity;
    }
}

void SPH::calculatePressureForce(const float dt, const auto start, const auto end)
{
    const float squareRadius = std::pow(_config.smoothingRadius, 2.0f);

    for (auto particleIt = start; particleIt != end; ++particleIt) {
        auto& particle = *particleIt;
        const float pressure = pressureFromDensity(particle._density);
        const float nearPressure = nearPressureFromDensity(particle._nearDensity);
        Vec3<float> pressureForce {};
        const auto originCell = getCell(particle);
        int neighborCount = 0;

        for (const auto offset : OFFSETS_3D) {
            const auto cell = originCell + offset;
            const uint32_t key = keyFromHash(hash(cell));
            uint32_t neighborIndex = _offsets[key];

            while (neighborIndex < _particles.size() && _keys[neighborIndex] == key) {
                ++neighborIndex;

                if (auto& neighbor = _particles[neighborIndex]; neighbor != particle) {

                    const auto distanceToNeighbor = neighbor._predicted - particle._predicted;

                    if (const float squareDistance = distanceToNeighbor * distanceToNeighbor;
                        squareDistance <= squareRadius) {
                        const float sharedPressure
                            = (pressure + pressureFromDensity(neighbor._density)) * 0.5f;
                        const float sharedNearPressure
                            = (nearPressure + nearPressureFromDensity(neighbor._density)) * 0.5f;

                        const float dstToNeighbor = std::sqrt(squareDistance);
                        const auto dirToNeighbor = dstToNeighbor > 1e-6f
                            ? distanceToNeighbor / dstToNeighbor
                            : Vec3<float> {};

                        pressureForce += dirToNeighbor * densityDerivative(dstToNeighbor)
                            * sharedPressure / neighbor._density;

                        pressureForce += dirToNeighbor * nearDensityDerivative(dstToNeighbor)
                            * sharedNearPressure / std::max(1e-6f, neighbor._nearDensity);

                        ++neighborCount;
                    }
                }
            }
        }

        const auto acceleration = pressureForce * (1.0f / std::max(1e-6f, particle._density));
        particle._velocity += acceleration * dt;

        // Airborne drag
        if (neighborCount < 8) {
            particle._velocity -= particle._velocity * dt * 0.75f;
        }
    }
}

void SPH::calculateViscosity(const std::vector<Vec3<float>>& velocitySnapshot, const float dt,
    const auto start, const auto end)
{
    const float squareRadius = std::pow(_config.smoothingRadius, 2.0f);

    for (auto particleIt = start; particleIt != end; ++particleIt) {
        uint32_t id = particleIt - _particles.begin();
        auto& particle = *particleIt;
        const auto originCell = getCell(particle);
        Vec3<float> viscosityForce {};
        const auto velocity = velocitySnapshot[id];

        for (const auto offset : OFFSETS_3D) {
            Vec3<int> cell = originCell + offset;
            const uint32_t key = keyFromHash(hash(cell));
            uint32_t neighborIndex = _offsets[key];

            while (neighborIndex < _particles.size() && _keys[neighborIndex] == key) {
                ++neighborIndex;

                if (auto& neighbor = _particles[neighborIndex]; neighbor != particle) {
                    const auto distanceToNeighbor = neighbor._predicted - particle._predicted;
                    const float squareDistance = distanceToNeighbor * distanceToNeighbor;

                    if (squareDistance <= squareRadius) {
                        const float distance = std::sqrt(squareDistance);
                        viscosityForce
                            += (velocitySnapshot[neighborIndex] - velocity) * poly6Kernel(distance);
                    }
                }
            }
        }

        _particles[id]._velocity += viscosityForce * _config.viscosityStrength * dt;
    }
}

void SPH::updatePositions(const float dt, const auto start, const auto end)
{
    for (auto particleIt = start; particleIt != end; ++particleIt) {
        auto& particle = *particleIt;
        particle._position += particle._velocity * dt;
        resolveCollisions(particle);
    }
}
