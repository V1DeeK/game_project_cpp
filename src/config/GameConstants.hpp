#pragma once

namespace game
{
inline constexpr float shipVisualScale = 0.45f;
inline constexpr int minAsteroidCount = 10;
inline constexpr int maxAsteroidCount = 15;
inline constexpr int baseAsteroidHitPoints = 2;
inline constexpr float asteroidMassRetention = 0.9f;
inline constexpr float asteroidSpeedAfterMerge = 0.5f;
inline constexpr int orbitSatelliteCount = 8;
inline constexpr int satelliteHitPoints = 3;
inline constexpr int firingPointCount = 36;
inline constexpr int firingRingCount = 2;
inline constexpr float firingRing2AngleOffsetDegrees = 5.f;
inline constexpr float firingRingSpacing = 55.f;
inline constexpr float orbitAngularSpeed = 1.5f;
inline constexpr int neutralShipCount = 4;
inline constexpr float neutralShipSizeMultiplier = 2.5f;
inline constexpr float neutralMoveSpeed = 10.f;
inline constexpr int enemySpawnCount = 4;
inline constexpr int targetAsteroidCount = (minAsteroidCount + maxAsteroidCount) / 2;
inline constexpr float firingRingOffset = 55.f;
inline constexpr float enemyMoveSpeed = 45.f;
inline constexpr float enemyFireInterval = 2.f;
} // namespace game
