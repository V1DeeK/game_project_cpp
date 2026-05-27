#pragma once

namespace game
{
// Math
inline constexpr float pi = 3.14159265f;
inline constexpr float twoPi = 6.2831853f;
inline constexpr float degreesPerRadian = 180.f / pi;
inline constexpr float radiansPerDegree = pi / 180.f;
inline constexpr float half = 0.5f;
inline constexpr float unitScale = 1.f;
inline constexpr float directionEpsilon = 0.0001f;
inline constexpr float screenOrigin = 0.f;
inline constexpr float centerDivisor = 2.f;

// Reference resolution & base sizes (scaled in GameContext)
inline constexpr float referenceWindowWidth = 800.f;
inline constexpr float referenceWindowHeight = 600.f;
inline constexpr float baseShipWidth = 16.f;
inline constexpr float baseShipHeight = 20.f;
inline constexpr float baseMoveSpeed = 250.f;
inline constexpr float baseEarthRadius = 80.f;
inline constexpr float baseBulletSpeed = 500.f;
inline constexpr float shipVisualScale = 0.45f;

// Application / loop
inline constexpr int targetFrameRate = 60;
inline constexpr float deltaTimeDivisor = 500.f;
inline constexpr float minDeltaTime = 1.f / deltaTimeDivisor;
inline constexpr float maxDeltaTime = 0.05f;
inline constexpr int initialEnemyCapacity = 32;
inline constexpr int initialBulletCapacity = 64;

// Shapes & drawing
inline constexpr int shipTriangleVertexCount = 3;
inline constexpr int triangleVertexNose = 0;
inline constexpr int triangleVertexBottomLeft = 1;
inline constexpr int triangleVertexBottomRight = 2;
inline constexpr float defaultOutlineThickness = 1.f;
inline constexpr float earthOutlineThickness = 4.f;
inline constexpr float shipBodyForwardFactor = 0.5f;
inline constexpr float shipRotationOffsetDegrees = 90.f;

// Screen edges (off-screen spawn)
inline constexpr int screenEdgeCount = 4;
inline constexpr int screenEdgeTop = 0;
inline constexpr int screenEdgeRight = 1;
inline constexpr int screenEdgeBottom = 2;
inline constexpr int screenEdgeLeft = 3;

// Damage & HP
inline constexpr int collisionDamage = 1;
inline constexpr int hitPointsDepleted = 0;
inline constexpr int initialMergeCount = 1;
inline constexpr int initialHitPoints = 0;
inline constexpr float initialFireCooldown = 0.f;
inline constexpr float initialOrbitAngle = 0.f;
inline constexpr float initialOrbitRadius = 0.f;
inline constexpr float initialAngularSpeed = 0.f;
inline constexpr int initialTargetRingIndex = 0;
inline constexpr int initialTargetPointIndex = 0;
inline constexpr int indexIncrement = 1;

// Asteroids
inline constexpr int minAsteroidCount = 10;
inline constexpr int maxAsteroidCount = 15;
inline constexpr int targetAsteroidCount = (minAsteroidCount + maxAsteroidCount) / centerDivisor;
inline constexpr int baseAsteroidHitPoints = 2;
inline constexpr float asteroidMassRetention = 0.9f;
inline constexpr float asteroidSpeedAfterMerge = 0.5f;
inline constexpr float asteroidRadiusMultiplier = 2.f;
inline constexpr float asteroidSpawnMarginExtra = 30.f;
inline constexpr float asteroidMinSpeed = 30.f;
inline constexpr float asteroidMaxSpeed = 60.f;

// Orbit satellites
inline constexpr int orbitSatelliteCount = 8;
inline constexpr int satelliteHitPoints = 3;
inline constexpr float orbitAngularSpeed = 1.5f;
inline constexpr float satelliteOrbitOffsetFromEarth = 24.f;
inline constexpr float satelliteRadiusBase = 5.f;
inline constexpr float satelliteHpBarWidthBase = 18.f;
inline constexpr float satelliteHpBarHeightBase = 3.f;
inline constexpr float satelliteHpBarVerticalOffsetBase = 7.f;

// Firing rings
inline constexpr int firingPointCount = 36;
inline constexpr int firingRingCount = 2;
inline constexpr float firingRing2AngleOffsetDegrees = 5.f;
inline constexpr float firingRingSpacing = 55.f;
inline constexpr float firingRingOffset = firingRingSpacing;
inline constexpr int noFiringRingIndex = -1;

// Neutral ships
inline constexpr int neutralShipCount = 4;
inline constexpr float neutralShipSizeMultiplier = 2.5f;
inline constexpr float neutralMoveSpeed = 10.f;
inline constexpr float neutralOffScreenRemoveMargin = 80.f;
inline constexpr float neutralSpawnAngleOffsetMin = -0.45f;
inline constexpr float neutralSpawnAngleOffsetMax = 0.45f;

// Enemies
inline constexpr int enemySpawnCount = 4;
inline constexpr float enemyMoveSpeed = 45.f;
inline constexpr float enemyFireInterval = 2.f;
inline constexpr float enemyArrivalDistanceFactor = 4.f;
inline constexpr int enemyBehaviorRandomMin = 0;
inline constexpr int enemyBehaviorRandomMax = 1;
inline constexpr int enemyBehaviorAttackSatellitesRoll = 0;

// Spawn margins
inline constexpr float offScreenSpawnMarginExtra = 40.f;

// Bullets
inline constexpr float bulletWidthBase = 2.f;
inline constexpr float bulletLengthBase = 5.f;
inline constexpr float bulletMuzzleOffsetFactor = 0.6f;
inline constexpr float bulletOffScreenMargin = 40.f;

// UI / start screen
inline constexpr float startScreenButtonWidthBase = 160.f;
inline constexpr float startScreenButtonHeightBase = 44.f;
inline constexpr float startScreenButtonCornerRadiusBase = 12.f;
inline constexpr float startScreenButtonGapBase = 14.f;
inline constexpr float startScreenButtonOffsetYBase = 120.f;
inline constexpr float startScreenButtonOffsetYExtra = 100.f;
inline constexpr float startScreenButtonFontSizeBase = 22.f;
inline constexpr int startScreenButtonCornerSegments = 8;

// Rounded-rectangle helper (ConvexShape)
inline constexpr int roundedRectangleCornerCount = 4;
inline constexpr int roundedRectangleExtraPoints = 4;

// UI / game over modal
inline constexpr int gameOverOverlayAlpha = 160;
inline constexpr float gameOverPanelWidthBase = 360.f;
inline constexpr float gameOverPanelHeightBase = 260.f;
inline constexpr float gameOverPanelCornerRadiusBase = 16.f;
inline constexpr float gameOverScoreFontSizeBase = 26.f;
inline constexpr float gameOverScoreLineSpacingBase = 36.f;
inline constexpr float gameOverButtonWidthBase = 160.f;
inline constexpr float gameOverButtonHeightBase = 44.f;
inline constexpr float gameOverButtonCornerRadiusBase = 12.f;
inline constexpr float gameOverButtonGapBase = 14.f;
inline constexpr float gameOverButtonOffsetYBase = 40.f;
inline constexpr float gameOverButtonFontSizeBase = 22.f;

// UI / record
inline constexpr float recordHudMarginX = 24.f;
inline constexpr float recordHudMarginY = 20.f;
inline constexpr float recordHudOffsetLeft = 100.f;
inline constexpr float recordHudFontSizeBase = 28.f;
inline constexpr char recordFileName[] = "record.txt";
} // namespace game
