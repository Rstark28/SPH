//
// Created by Robert Stark on 2/16/26.
//

#ifndef RULES_H
#define RULES_H

#include "Math/Vec.h"
#include "Particle.h"
#include <algorithm>
#include <random>
#include <vector>

constexpr float PI = 3.1415926f;

inline float toRadians(const float degrees) noexcept
{
    return degrees * static_cast<float>(PI) / 180.0f;
}

inline std::vector<Particle> spawnParticlesInBox(
    const size_t count, const float boxSize, const float margin, const float minHeightRatio)
{
    std::vector<Particle> particles;
    particles.reserve(count);

    const float halfBox = boxSize * 0.5f;
    const float clampedMargin = std::clamp(margin, 0.0f, halfBox);
    const float maxY = halfBox - clampedMargin;
    float minY = std::max(-halfBox + clampedMargin, minHeightRatio * halfBox);
    if (minY > maxY) {
        minY = maxY;
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> xDist(-halfBox + clampedMargin, halfBox - clampedMargin);
    std::uniform_real_distribution<float> yDist(minY, maxY);
    std::uniform_real_distribution<float> zDist(-halfBox + clampedMargin, halfBox - clampedMargin);

    for (size_t i = 0; i < count; ++i) {
        Vec3<float> position { xDist(rng), yDist(rng), zDist(rng) };
        Vec3<float> velocity { 0.0f, 0.0f, 0.0f };
        particles.emplace_back(position, velocity);
    }

    return particles;
}

#endif // RULES_H
