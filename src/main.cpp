#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

struct GameContext
{
	float windowWidth = 800.f;
	float windowHeight = 600.f;
	float shipWidth = 16.f;
	float shipHeight = 20.f;
	float moveSpeed = 250.f;
	float earthRadius = 80.f;
	float scale = 1.f;
	float bulletSpeed = 500.f;
};

struct Asteroid
{
	sf::CircleShape shape;
	sf::Vector2f velocity;
	int hp = 2;
	int maxHp = 2;
	int mergeCount = 1;
};

struct OrbitSatellite
{
	sf::CircleShape shape;
	float angle = 0.f;
	float angularSpeed = 0.f;
	float orbitRadius = 0.f;
	int hp = 2;
	int maxHp = 2;
};

struct NeutralShip
{
	sf::ConvexShape shape;
	sf::Vector2f velocity;
};

struct Bullet
{
	sf::RectangleShape shape;
	sf::Vector2f velocity;
};

struct EnemyShip
{
	sf::ConvexShape shape;
	sf::Vector2f targetPosition;
	int targetRingIndex = 0;
	int targetPointIndex = 0;
	float fireCooldown = 0.f;
};

namespace
{
constexpr float shipVisualScale = 0.45f;
constexpr int minAsteroidCount = 10;
constexpr int maxAsteroidCount = 15;
constexpr int baseAsteroidHitPoints = 2;
constexpr float asteroidMassRetention = 0.9f;
constexpr float asteroidSpeedAfterMerge = 0.5f;
constexpr int orbitSatelliteCount = 8;
constexpr int satelliteHitPoints = 3;
constexpr int firingPointCount = 36;
constexpr int firingRingCount = 2;
constexpr float firingRing2AngleOffsetDegrees = 5.f;
constexpr float firingRingSpacing = 55.f;
constexpr float orbitAngularSpeed = 1.5f;
constexpr int neutralShipCount = 4;
constexpr float neutralShipSizeMultiplier = 2.5f;
constexpr float neutralMoveSpeed = 10.f;
constexpr int enemySpawnCount = 4;
constexpr int targetAsteroidCount = (minAsteroidCount + maxAsteroidCount) / 2;
constexpr float firingRingOffset = 55.f;
constexpr float enemyMoveSpeed = 45.f;
constexpr float enemyFireInterval = 2.f;

constexpr sf::Color kColorPlayerShipFill(50, 120, 230);
constexpr sf::Color kColorPlayerShipOutline(120, 180, 255);
constexpr sf::Color kColorSatelliteFill(55, 95, 220);
constexpr sf::Color kColorSatelliteOutline(100, 150, 255);
constexpr sf::Color kColorAsteroidFill(139, 90, 43);
constexpr sf::Color kColorAsteroidOutline(100, 65, 30);
constexpr sf::Color kColorNeutralFill(35, 130, 55);
constexpr sf::Color kColorNeutralOutline(20, 90, 40);
constexpr sf::Color kColorEnemyFill(200, 45, 45);
constexpr sf::Color kColorEnemyOutline(255, 90, 90);
constexpr sf::Color kColorBullet(255, 220, 50);
constexpr sf::Color kColorHpBackground(90, 20, 20);
constexpr sf::Color kColorHpForeground(0, 200, 0);
constexpr sf::Color kColorHpDamaged(200, 40, 40);

bool IsMoveKeyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Keyboard::isKeyPressed(scancode);
}

GameContext CreateGameContext(const sf::VideoMode& desktopMode)
{
	GameContext context;
	context.windowWidth = static_cast<float>(desktopMode.size.x);
	context.windowHeight = static_cast<float>(desktopMode.size.y);
	context.scale = std::min(context.windowWidth / 800.f, context.windowHeight / 600.f);

	context.shipWidth = 16.f * context.scale * shipVisualScale;
	context.shipHeight = 20.f * context.scale * shipVisualScale;
	context.moveSpeed = 250.f * context.scale;
	context.earthRadius = 80.f * context.scale;
	context.bulletSpeed = 500.f * context.scale;

	return context;
}

sf::Vector2f GetEarthCenter(const GameContext& context)
{
	return sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f);
}

float GetSatelliteRadius(const GameContext& context)
{
	return 5.f * context.scale;
}

sf::Vector2f GetNeutralShipSize(const GameContext& context)
{
	return sf::Vector2f(
		context.shipWidth * neutralShipSizeMultiplier,
		context.shipHeight * neutralShipSizeMultiplier);
}

float GetSatelliteOrbitRadius(const GameContext& context)
{
	return context.earthRadius + 24.f * context.scale;
}

float GetFiringRingRadius(const GameContext& context, int ringIndex)
{
	return GetSatelliteOrbitRadius(context)
		+ (firingRingOffset + firingRingSpacing * static_cast<float>(ringIndex)) * context.scale;
}

sf::Vector2f GetFiringPointPosition(
	sf::Vector2f earthCenter,
	const GameContext& context,
	int ringIndex,
	int pointIndex)
{
	const float firingRadius = GetFiringRingRadius(context, ringIndex);
	const float angleStep = 6.2831853f / static_cast<float>(firingPointCount);
	const float ringAngleOffset = firingRing2AngleOffsetDegrees * 3.14159265f / 180.f
		* static_cast<float>(ringIndex);
	const float angle = angleStep * static_cast<float>(pointIndex) + ringAngleOffset;
	return sf::Vector2f(
		earthCenter.x + std::cos(angle) * firingRadius,
		earthCenter.y + std::sin(angle) * firingRadius);
}

bool IsFiringSlotOccupied(int ringIndex, int pointIndex, const std::vector<EnemyShip>& enemies)
{
	for (const auto& enemy : enemies)
	{
		if (enemy.targetRingIndex == ringIndex && enemy.targetPointIndex == pointIndex)
		{
			return true;
		}
	}
	return false;
}

bool HasFreeFiringSlotOnRing(int ringIndex, const std::vector<EnemyShip>& enemies)
{
	for (int pointIndex = 0; pointIndex < firingPointCount; ++pointIndex)
	{
		if (!IsFiringSlotOccupied(ringIndex, pointIndex, enemies))
		{
			return true;
		}
	}
	return false;
}

struct FiringSlotAssignment
{
	int ringIndex = 0;
	int pointIndex = 0;
};

FiringSlotAssignment FindNearestFiringSlot(
	sf::Vector2f spawnPosition,
	sf::Vector2f earthCenter,
	const GameContext& context,
	const std::vector<EnemyShip>& enemies)
{
	FiringSlotAssignment result;
	int searchRingCount = firingRingCount;

	for (int ringIndex = 0; ringIndex < firingRingCount; ++ringIndex)
	{
		if (HasFreeFiringSlotOnRing(ringIndex, enemies))
		{
			searchRingCount = ringIndex + 1;
			break;
		}
	}

	int bestFreeRing = -1;
	int bestFreePoint = 0;
	float bestFreeDistance = std::numeric_limits<float>::max();
	int bestAnyRing = 0;
	int bestAnyPoint = 0;
	float bestAnyDistance = std::numeric_limits<float>::max();

	for (int ringIndex = 0; ringIndex < searchRingCount; ++ringIndex)
	{
		for (int pointIndex = 0; pointIndex < firingPointCount; ++pointIndex)
		{
			const sf::Vector2f slotPosition = GetFiringPointPosition(earthCenter, context, ringIndex, pointIndex);
			const sf::Vector2f delta = slotPosition - spawnPosition;
			const float distance = delta.x * delta.x + delta.y * delta.y;

			if (distance < bestAnyDistance)
			{
				bestAnyDistance = distance;
				bestAnyRing = ringIndex;
				bestAnyPoint = pointIndex;
			}

			if (!IsFiringSlotOccupied(ringIndex, pointIndex, enemies) && distance < bestFreeDistance)
			{
				bestFreeDistance = distance;
				bestFreeRing = ringIndex;
				bestFreePoint = pointIndex;
			}
		}
	}

	if (bestFreeRing >= 0)
	{
		result.ringIndex = bestFreeRing;
		result.pointIndex = bestFreePoint;
	}
	else
	{
		result.ringIndex = bestAnyRing;
		result.pointIndex = bestAnyPoint;
	}

	return result;
}

void RotateShipTowardPoint(sf::ConvexShape& ship, sf::Vector2f shipCenter, sf::Vector2f targetPoint)
{
	sf::Vector2f direction = targetPoint - shipCenter;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= 0.0001f)
	{
		return;
	}

	const float angleDegrees = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f + 90.f;
	ship.setRotation(sf::degrees(angleDegrees));
}

sf::ConvexShape CreateEnemyShip(const GameContext& context)
{
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight / 2.f));
	ship.setFillColor(kColorEnemyFill);
	ship.setOutlineColor(kColorEnemyOutline);
	ship.setOutlineThickness(1.f);
	return ship;
}

sf::Vector2f CreateSpawnPositionFromEdge(int edge, const GameContext& context, float spawnMargin, std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(0.f, context.windowWidth);
	std::uniform_real_distribution<float> positionY(0.f, context.windowHeight);

	switch (edge)
	{
	case 0:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case 1:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case 2:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}

void SpawnEnemiesFromAllSides(
	std::vector<EnemyShip>& enemies,
	const GameContext& context,
	sf::Vector2f earthCenter,
	std::mt19937& rng)
{
	const float spawnMargin = std::max(context.shipWidth, context.shipHeight) + 40.f * context.scale;

	for (int enemyIndex = 0; enemyIndex < enemySpawnCount; ++enemyIndex)
	{
		const int edge = enemyIndex;
		const sf::Vector2f spawnPosition = CreateSpawnPositionFromEdge(edge, context, spawnMargin, rng);

		const FiringSlotAssignment slot = FindNearestFiringSlot(spawnPosition, earthCenter, context, enemies);

		EnemyShip enemy;
		enemy.shape = CreateEnemyShip(context);
		enemy.shape.setPosition(spawnPosition);
		enemy.targetRingIndex = slot.ringIndex;
		enemy.targetPointIndex = slot.pointIndex;
		enemy.targetPosition = GetFiringPointPosition(earthCenter, context, slot.ringIndex, slot.pointIndex);
		RotateShipTowardPoint(enemy.shape, spawnPosition, earthCenter);
		enemies.push_back(enemy);
	}
}

bool IsIntersectingScreen(const sf::FloatRect& bounds, const GameContext& context)
{
	return bounds.position.x < context.windowWidth
		&& bounds.position.x + bounds.size.x > 0.f
		&& bounds.position.y < context.windowHeight
		&& bounds.position.y + bounds.size.y > 0.f;
}

bool IsShapeOnScreen(const sf::Shape& shape, const GameContext& context)
{
	return IsIntersectingScreen(shape.getGlobalBounds(), context);
}

bool RectsIntersect(const sf::FloatRect& first, const sf::FloatRect& second)
{
	return first.position.x < second.position.x + second.size.x
		&& first.position.x + first.size.x > second.position.x
		&& first.position.y < second.position.y + second.size.y
		&& first.position.y + first.size.y > second.position.y;
}

bool BulletIntersectsConvexShape(const Bullet& bullet, const sf::ConvexShape& shape)
{
	const sf::FloatRect bulletBounds = bullet.shape.getGlobalBounds();
	const sf::Vector2f bulletCenter(
		bulletBounds.position.x + bulletBounds.size.x / 2.f,
		bulletBounds.position.y + bulletBounds.size.y / 2.f);
	return shape.getGlobalBounds().contains(bulletCenter);
}

void ProcessBulletNeutralCollisions(
	std::vector<Bullet>& bullets,
	std::vector<NeutralShip>& neutrals,
	std::vector<EnemyShip>& enemies,
	const GameContext& context,
	sf::Vector2f earthCenter,
	std::mt19937& rng)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		bool bulletHit = false;

		for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size(); ++neutralIndex)
		{
			if (!IsShapeOnScreen(neutrals[neutralIndex].shape, context))
			{
				continue;
			}

			if (!BulletIntersectsConvexShape(bullets[bulletIndex], neutrals[neutralIndex].shape))
			{
				continue;
			}

			neutrals.erase(neutrals.begin() + static_cast<std::ptrdiff_t>(neutralIndex));
			SpawnEnemiesFromAllSides(enemies, context, earthCenter, rng);
			bulletHit = true;
			break;
		}

		if (bulletHit)
		{
			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
		}
		else
		{
			++bulletIndex;
		}
	}
}

sf::ConvexShape CreatePlayerShip(const GameContext& context)
{
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight / 2.f));
	ship.setFillColor(kColorPlayerShipFill);
	ship.setOutlineColor(kColorPlayerShipOutline);
	ship.setOutlineThickness(1.f);
	return ship;
}

sf::CircleShape CreateEarth(const GameContext& context)
{
	sf::CircleShape earth(context.earthRadius);
	earth.setOrigin(sf::Vector2f(context.earthRadius, context.earthRadius));
	earth.setPosition(GetEarthCenter(context));
	earth.setFillColor(sf::Color(25, 90, 170));
	earth.setOutlineColor(sf::Color(45, 150, 70));
	earth.setOutlineThickness(4.f);
	return earth;
}

void SetOrbitSatellitePosition(OrbitSatellite& satellite, sf::Vector2f earthCenter)
{
	const float x = earthCenter.x + std::cos(satellite.angle) * satellite.orbitRadius;
	const float y = earthCenter.y + std::sin(satellite.angle) * satellite.orbitRadius;
	satellite.shape.setPosition(sf::Vector2f(x, y));
}

std::vector<OrbitSatellite> CreateOrbitSatellites(const GameContext& context)
{
	const float satelliteRadius = GetSatelliteRadius(context);
	const float orbitRadius = context.earthRadius + 24.f * context.scale;
	const sf::Vector2f earthCenter = GetEarthCenter(context);
	const float angleStep = 6.2831853f / static_cast<float>(orbitSatelliteCount);

	std::vector<OrbitSatellite> satellites;
	satellites.reserve(orbitSatelliteCount);

	for (int i = 0; i < orbitSatelliteCount; ++i)
	{
		OrbitSatellite satellite;
		satellite.angle = angleStep * static_cast<float>(i);
		satellite.angularSpeed = orbitAngularSpeed;
		satellite.orbitRadius = orbitRadius;
		satellite.hp = satelliteHitPoints;
		satellite.maxHp = satelliteHitPoints;
		satellite.shape = sf::CircleShape(satelliteRadius);
		satellite.shape.setOrigin(sf::Vector2f(satelliteRadius, satelliteRadius));
		satellite.shape.setFillColor(kColorSatelliteFill);
		satellite.shape.setOutlineColor(kColorSatelliteOutline);
		satellite.shape.setOutlineThickness(1.f);
		SetOrbitSatellitePosition(satellite, earthCenter);
		satellites.push_back(satellite);
	}

	return satellites;
}

void ClampCircleToScreen(sf::CircleShape& circle, const GameContext& context)
{
	const float radius = circle.getRadius();
	sf::Vector2f position = circle.getPosition();
	position.x = std::clamp(position.x, radius, context.windowWidth - radius);
	position.y = std::clamp(position.y, radius, context.windowHeight - radius);
	circle.setPosition(position);
}

void UpdateOrbitSatellites(
	std::vector<OrbitSatellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	const GameContext& context)
{
	for (auto& satellite : satellites)
	{
		satellite.angle += satellite.angularSpeed * deltaTime;
		SetOrbitSatellitePosition(satellite, earthCenter);
		ClampCircleToScreen(satellite.shape, context);
	}
}

void DrawSatelliteHpBar(sf::RenderWindow& window, const OrbitSatellite& satellite, const GameContext& context)
{
	const float radius = satellite.shape.getRadius();
	const sf::Vector2f center = satellite.shape.getPosition();
	const float barWidth = 18.f * context.scale;
	const float barHeight = 3.f * context.scale;
	const sf::Vector2f barPosition(center.x - barWidth / 2.f, center.y - radius - 7.f * context.scale);

	sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
	background.setPosition(barPosition);
	background.setFillColor(kColorHpBackground);

	const float hpFraction = static_cast<float>(satellite.hp) / static_cast<float>(satellite.maxHp);
	const float foregroundWidth = barWidth * hpFraction;

	sf::RectangleShape foreground(sf::Vector2f(foregroundWidth, barHeight));
	foreground.setPosition(barPosition);
	if (satellite.hp == satellite.maxHp)
	{
		foreground.setFillColor(kColorHpForeground);
	}
	else if (satellite.hp > 0)
	{
		foreground.setFillColor(kColorHpDamaged);
	}

	window.draw(background);
	if (foregroundWidth > 0.f)
	{
		window.draw(foreground);
	}
}

sf::Vector2f GetShipForwardDirection(const sf::ConvexShape& ship, const GameContext& context)
{
	const sf::Transform transform = ship.getTransform();
	const sf::Vector2f nose = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
	const sf::Vector2f body = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight * 0.5f));
	sf::Vector2f direction = nose - body;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.0001f)
	{
		direction /= length;
	}
	return direction;
}

sf::Vector2f GetShipNosePosition(const sf::ConvexShape& ship, const GameContext& context)
{
	return ship.getTransform().transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
}

Bullet CreateBulletFromShip(const sf::ConvexShape& ship, const GameContext& context)
{
	const sf::Vector2f direction = GetShipForwardDirection(ship, context);
	const sf::Vector2f nosePosition = GetShipNosePosition(ship, context);

	const float bulletWidth = 2.f * context.scale;
	const float bulletLength = 5.f * context.scale;

	sf::RectangleShape shape(sf::Vector2f(bulletWidth, bulletLength));
	shape.setOrigin(sf::Vector2f(bulletWidth / 2.f, bulletLength / 2.f));
	shape.setPosition(nosePosition + direction * (bulletLength * 0.6f));
	shape.setFillColor(kColorBullet);

	const float angleDegrees = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f + 90.f;
	shape.setRotation(sf::degrees(angleDegrees));

	return Bullet{ shape, direction * context.bulletSpeed };
}

sf::Vector2f CreateOffScreenSpawnPosition(
	const GameContext& context,
	float spawnMargin,
	std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(0.f, context.windowWidth);
	std::uniform_real_distribution<float> positionY(0.f, context.windowHeight);
	std::uniform_int_distribution<int> edgeDist(0, 3);

	switch (edgeDist(rng))
	{
	case 0:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case 1:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case 2:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}

Asteroid CreateRandomAsteroid(const GameContext& context, std::mt19937& rng)
{
	const float radius = GetSatelliteRadius(context) * 2.f;
	const float spawnMargin = radius + 30.f * context.scale;

	std::uniform_real_distribution<float> speed(30.f, 60.f);
	std::uniform_real_distribution<float> direction(0.f, 6.2831853f);

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float asteroidSpeed = speed(rng) * context.scale;
	const float angle = direction(rng);
	const sf::Vector2f velocity(std::cos(angle) * asteroidSpeed, std::sin(angle) * asteroidSpeed);

	sf::CircleShape shape(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
	shape.setPosition(position);
	shape.setFillColor(kColorAsteroidFill);
	shape.setOutlineColor(kColorAsteroidOutline);
	shape.setOutlineThickness(1.f);

	return Asteroid{ shape, velocity, baseAsteroidHitPoints, baseAsteroidHitPoints, 1 };
}

void UpdateAsteroidDamageVisual(Asteroid& asteroid)
{
	if (asteroid.hp < asteroid.maxHp)
	{
		asteroid.shape.setFillColor(sf::Color(200, 130, 70));
	}
	else
	{
		asteroid.shape.setFillColor(kColorAsteroidFill);
	}
}

bool CircleContainsPoint(sf::Vector2f center, float radius, sf::Vector2f point)
{
	const float dx = point.x - center.x;
	const float dy = point.y - center.y;
	return dx * dx + dy * dy <= radius * radius;
}

bool CirclesIntersect(sf::Vector2f centerA, float radiusA, sf::Vector2f centerB, float radiusB)
{
	const float dx = centerA.x - centerB.x;
	const float dy = centerA.y - centerB.y;
	const float combinedRadius = radiusA + radiusB;
	return dx * dx + dy * dy <= combinedRadius * combinedRadius;
}

bool CircleIntersectsRect(sf::Vector2f circleCenter, float radius, const sf::FloatRect& rect)
{
	const float closestX = std::clamp(circleCenter.x, rect.position.x, rect.position.x + rect.size.x);
	const float closestY = std::clamp(circleCenter.y, rect.position.y, rect.position.y + rect.size.y);
	const float dx = circleCenter.x - closestX;
	const float dy = circleCenter.y - closestY;
	return dx * dx + dy * dy <= radius * radius;
}

bool ProcessAsteroidPlayerCollision(
	std::vector<Asteroid>& asteroids,
	const sf::ConvexShape& playerShip,
	const GameContext& context)
{
	const sf::FloatRect playerBounds = playerShip.getGlobalBounds();

	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size(); ++asteroidIndex)
	{
		if (!IsShapeOnScreen(asteroids[asteroidIndex].shape, context))
		{
			continue;
		}

		const sf::Vector2f asteroidCenter = asteroids[asteroidIndex].shape.getPosition();
		const float asteroidRadius = asteroids[asteroidIndex].shape.getRadius();

		if (CircleIntersectsRect(asteroidCenter, asteroidRadius, playerBounds))
		{
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			return true;
		}
	}

	return false;
}

Asteroid MergeAsteroids(const Asteroid& first, const Asteroid& second)
{
	const float radiusA = first.shape.getRadius();
	const float radiusB = second.shape.getRadius();
	const float massA = radiusA;
	const float massB = radiusB;
	const float totalMassBeforeLoss = massA + massB;
	const float mergedMass = totalMassBeforeLoss * asteroidMassRetention;

	const sf::Vector2f positionA = first.shape.getPosition();
	const sf::Vector2f positionB = second.shape.getPosition();
	const sf::Vector2f mergedPosition = (massA * positionA + massB * positionB) / totalMassBeforeLoss;

	sf::Vector2f mergedVelocity = (massA * first.velocity + massB * second.velocity) / totalMassBeforeLoss;
	mergedVelocity *= asteroidSpeedAfterMerge;

	const float mergedRadius = mergedMass;
	const int mergedCount = first.mergeCount + second.mergeCount;

	sf::CircleShape shape(mergedRadius);
	shape.setOrigin(sf::Vector2f(mergedRadius, mergedRadius));
	shape.setPosition(mergedPosition);
	shape.setFillColor(kColorAsteroidFill);
	shape.setOutlineColor(kColorAsteroidOutline);
	shape.setOutlineThickness(1.f);

	return Asteroid{ shape, mergedVelocity, baseAsteroidHitPoints * mergedCount, baseAsteroidHitPoints * mergedCount, mergedCount };
}

void ProcessAsteroidMerges(std::vector<Asteroid>& asteroids)
{
	for (std::size_t firstIndex = 0; firstIndex < asteroids.size(); ++firstIndex)
	{
		for (std::size_t secondIndex = firstIndex + 1; secondIndex < asteroids.size();)
		{
			const sf::Vector2f centerA = asteroids[firstIndex].shape.getPosition();
			const float radiusA = asteroids[firstIndex].shape.getRadius();
			const sf::Vector2f centerB = asteroids[secondIndex].shape.getPosition();
			const float radiusB = asteroids[secondIndex].shape.getRadius();

			if (!CirclesIntersect(centerA, radiusA, centerB, radiusB))
			{
				++secondIndex;
				continue;
			}

			asteroids[firstIndex] = MergeAsteroids(asteroids[firstIndex], asteroids[secondIndex]);
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(secondIndex));
		}
	}
}

void ProcessAsteroidNeutralCollisions(
	std::vector<Asteroid>& asteroids,
	std::vector<NeutralShip>& neutrals,
	const GameContext& context)
{
	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size();)
	{
		bool asteroidDestroyed = false;
		const sf::Vector2f asteroidCenter = asteroids[asteroidIndex].shape.getPosition();
		const float asteroidRadius = asteroids[asteroidIndex].shape.getRadius();

		if (!IsShapeOnScreen(asteroids[asteroidIndex].shape, context))
		{
			++asteroidIndex;
			continue;
		}

		for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size(); ++neutralIndex)
		{
			if (!IsShapeOnScreen(neutrals[neutralIndex].shape, context))
			{
				continue;
			}

			if (!CircleIntersectsRect(
					asteroidCenter,
					asteroidRadius,
					neutrals[neutralIndex].shape.getGlobalBounds()))
			{
				continue;
			}

			neutrals.erase(neutrals.begin() + static_cast<std::ptrdiff_t>(neutralIndex));
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			asteroidDestroyed = true;
			break;
		}

		if (!asteroidDestroyed)
		{
			++asteroidIndex;
		}
	}
}

void ProcessAsteroidEnemyCollisions(
	std::vector<Asteroid>& asteroids,
	std::vector<EnemyShip>& enemies,
	const GameContext& context)
{
	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size();)
	{
		bool asteroidDestroyed = false;
		const sf::Vector2f asteroidCenter = asteroids[asteroidIndex].shape.getPosition();
		const float asteroidRadius = asteroids[asteroidIndex].shape.getRadius();

		if (!IsShapeOnScreen(asteroids[asteroidIndex].shape, context))
		{
			++asteroidIndex;
			continue;
		}

		for (std::size_t enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex)
		{
			if (!IsShapeOnScreen(enemies[enemyIndex].shape, context))
			{
				continue;
			}

			if (!CircleIntersectsRect(asteroidCenter, asteroidRadius, enemies[enemyIndex].shape.getGlobalBounds()))
			{
				continue;
			}

			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			asteroidDestroyed = true;
			break;
		}

		if (!asteroidDestroyed)
		{
			++asteroidIndex;
		}
	}
}

void ProcessAsteroidSatelliteCollisions(std::vector<Asteroid>& asteroids, std::vector<OrbitSatellite>& satellites)
{
	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size();)
	{
		const sf::Vector2f asteroidCenter = asteroids[asteroidIndex].shape.getPosition();
		const float asteroidRadius = asteroids[asteroidIndex].shape.getRadius();

		bool asteroidDestroyed = false;

		for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
		{
			const sf::Vector2f satelliteCenter = satellites[satelliteIndex].shape.getPosition();
			const float satelliteRadius = satellites[satelliteIndex].shape.getRadius();

			if (!CirclesIntersect(asteroidCenter, asteroidRadius, satelliteCenter, satelliteRadius))
			{
				continue;
			}

			satellites.erase(satellites.begin() + static_cast<std::ptrdiff_t>(satelliteIndex));
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			asteroidDestroyed = true;
			break;
		}

		if (!asteroidDestroyed)
		{
			++asteroidIndex;
		}
	}
}

std::optional<sf::Vector2f> FindNearestSatellitePosition(
	sf::Vector2f fromPosition,
	const std::vector<OrbitSatellite>& satellites)
{
	if (satellites.empty())
	{
		return std::nullopt;
	}

	std::optional<sf::Vector2f> nearestPosition;
	float nearestDistanceSquared = std::numeric_limits<float>::max();

	for (const auto& satellite : satellites)
	{
		const sf::Vector2f satellitePosition = satellite.shape.getPosition();
		const sf::Vector2f delta = satellitePosition - fromPosition;
		const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestPosition = satellitePosition;
		}
	}

	return nearestPosition;
}

void ProcessEnemyEnemyCollisions(std::vector<EnemyShip>& enemies, const GameContext& context)
{
	for (std::size_t firstIndex = 0; firstIndex < enemies.size();)
	{
		bool pairRemoved = false;

		for (std::size_t secondIndex = firstIndex + 1; secondIndex < enemies.size(); ++secondIndex)
		{
			if (!IsShapeOnScreen(enemies[firstIndex].shape, context)
				|| !IsShapeOnScreen(enemies[secondIndex].shape, context))
			{
				continue;
			}

			const sf::FloatRect firstBounds = enemies[firstIndex].shape.getGlobalBounds();
			const sf::FloatRect secondBounds = enemies[secondIndex].shape.getGlobalBounds();
			if (!RectsIntersect(firstBounds, secondBounds))
			{
				continue;
			}

			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(secondIndex));
			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(firstIndex));
			pairRemoved = true;
			break;
		}

		if (!pairRemoved)
		{
			++firstIndex;
		}
	}
}

bool ProcessEnemyPlayerCollision(
	std::vector<EnemyShip>& enemies,
	const sf::ConvexShape& playerShip,
	const GameContext& context)
{
	const sf::FloatRect playerBounds = playerShip.getGlobalBounds();

	for (std::size_t enemyIndex = 0; enemyIndex < enemies.size();)
	{
		if (!IsShapeOnScreen(enemies[enemyIndex].shape, context))
		{
			++enemyIndex;
			continue;
		}

		if (!RectsIntersect(enemies[enemyIndex].shape.getGlobalBounds(), playerBounds))
		{
			++enemyIndex;
			continue;
		}

		enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
		return true;
	}

	return false;
}

void ProcessBulletEnemyCollisions(
	std::vector<Bullet>& bullets,
	std::vector<EnemyShip>& enemies,
	const GameContext& context)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		bool bulletHit = false;

		for (std::size_t enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex)
		{
			if (!IsShapeOnScreen(enemies[enemyIndex].shape, context))
			{
				continue;
			}

			if (!BulletIntersectsConvexShape(bullets[bulletIndex], enemies[enemyIndex].shape))
			{
				continue;
			}

			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
			bulletHit = true;
			break;
		}

		if (bulletHit)
		{
			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
		}
		else
		{
			++bulletIndex;
		}
	}
}

void ProcessBulletSatelliteCollisions(std::vector<Bullet>& bullets, std::vector<OrbitSatellite>& satellites)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		const sf::FloatRect bulletBounds = bullets[bulletIndex].shape.getGlobalBounds();
		const sf::Vector2f bulletCenter(
			bulletBounds.position.x + bulletBounds.size.x / 2.f,
			bulletBounds.position.y + bulletBounds.size.y / 2.f);

		bool bulletHit = false;

		for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
		{
			const sf::Vector2f satelliteCenter = satellites[satelliteIndex].shape.getPosition();
			const float satelliteRadius = satellites[satelliteIndex].shape.getRadius();

			if (!CircleContainsPoint(satelliteCenter, satelliteRadius, bulletCenter))
			{
				continue;
			}

			satellites[satelliteIndex].hp -= 1;
			if (satellites[satelliteIndex].hp <= 0)
			{
				satellites.erase(satellites.begin() + static_cast<std::ptrdiff_t>(satelliteIndex));
			}

			bulletHit = true;
			break;
		}

		if (bulletHit)
		{
			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
		}
		else
		{
			++bulletIndex;
		}
	}
}

void ProcessBulletAsteroidCollisions(
	std::vector<Bullet>& bullets,
	std::vector<Asteroid>& asteroids,
	const GameContext& context)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		const sf::FloatRect bulletBounds = bullets[bulletIndex].shape.getGlobalBounds();
		const sf::Vector2f bulletCenter(
			bulletBounds.position.x + bulletBounds.size.x / 2.f,
			bulletBounds.position.y + bulletBounds.size.y / 2.f);

		bool bulletHit = false;

		for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size(); ++asteroidIndex)
		{
			if (!IsShapeOnScreen(asteroids[asteroidIndex].shape, context))
			{
				continue;
			}

			const sf::Vector2f asteroidCenter = asteroids[asteroidIndex].shape.getPosition();
			const float asteroidRadius = asteroids[asteroidIndex].shape.getRadius();

			if (!CircleContainsPoint(asteroidCenter, asteroidRadius, bulletCenter))
			{
				continue;
			}

			asteroids[asteroidIndex].hp -= 1;
			if (asteroids[asteroidIndex].hp <= 0)
			{
				asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			}
			else
			{
				UpdateAsteroidDamageVisual(asteroids[asteroidIndex]);
			}

			bulletHit = true;
			break;
		}

		if (bulletHit)
		{
			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
		}
		else
		{
			++bulletIndex;
		}
	}
}

std::vector<Asteroid> CreateInitialAsteroids(const GameContext& context, std::mt19937& rng)
{
	std::vector<Asteroid> asteroids;
	asteroids.reserve(static_cast<std::size_t>(targetAsteroidCount));
	for (int i = 0; i < targetAsteroidCount; ++i)
	{
		asteroids.push_back(CreateRandomAsteroid(context, rng));
	}
	return asteroids;
}

sf::ConvexShape CreateNeutralTriangleShape(const GameContext& context)
{
	const sf::Vector2f size = GetNeutralShipSize(context);
	const float width = size.x;
	const float height = size.y;

	sf::ConvexShape shape(3);
	shape.setPoint(0, sf::Vector2f(width / 2.f, 0.f));
	shape.setPoint(1, sf::Vector2f(0.f, height));
	shape.setPoint(2, sf::Vector2f(width, height));
	shape.setOrigin(sf::Vector2f(width / 2.f, height / 2.f));
	shape.setFillColor(kColorNeutralFill);
	shape.setOutlineColor(kColorNeutralOutline);
	shape.setOutlineThickness(1.f);
	return shape;
}

sf::Vector2f CreateInboundVelocity(sf::Vector2f spawnPosition, const GameContext& context, float speed, std::mt19937& rng)
{
	const sf::Vector2f screenCenter(context.windowWidth / 2.f, context.windowHeight / 2.f);
	sf::Vector2f direction = screenCenter - spawnPosition;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.0001f)
	{
		direction /= length;
	}

	std::uniform_real_distribution<float> angleOffset(-0.45f, 0.45f);
	const float offsetAngle = angleOffset(rng);
	const float cosA = std::cos(offsetAngle);
	const float sinA = std::sin(offsetAngle);
	const sf::Vector2f rotated(
		direction.x * cosA - direction.y * sinA,
		direction.x * sinA + direction.y * cosA);

	return rotated * speed;
}

bool IsFullyOutsideScreen(const sf::FloatRect& bounds, const GameContext& context, float margin)
{
	return bounds.position.x + bounds.size.x < -margin
		|| bounds.position.x > context.windowWidth + margin
		|| bounds.position.y + bounds.size.y < -margin
		|| bounds.position.y > context.windowHeight + margin;
}

NeutralShip CreateRandomNeutralShip(const GameContext& context, std::mt19937& rng)
{
	const sf::Vector2f shipSize = GetNeutralShipSize(context);
	const float spawnMargin = std::max(shipSize.x, shipSize.y) + 40.f * context.scale;

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float speed = neutralMoveSpeed * context.scale;
	const sf::Vector2f velocity = CreateInboundVelocity(position, context, speed, rng);

	sf::ConvexShape shape = CreateNeutralTriangleShape(context);
	shape.setPosition(position);

	const float angleDegrees = std::atan2(velocity.y, velocity.x) * 180.f / 3.14159265f + 90.f;
	shape.setRotation(sf::degrees(angleDegrees));

	return NeutralShip{ shape, velocity };
}

std::vector<NeutralShip> CreateNeutralShips(const GameContext& context, std::mt19937& rng)
{
	std::vector<NeutralShip> neutrals;
	neutrals.reserve(neutralShipCount);
	for (int i = 0; i < neutralShipCount; ++i)
	{
		neutrals.push_back(CreateRandomNeutralShip(context, rng));
	}
	return neutrals;
}

void KeepInsideScreen(sf::Shape& shape, const GameContext& context, sf::Vector2f* velocity)
{
	sf::FloatRect bounds = shape.getGlobalBounds();

	if (bounds.position.x < 0.f)
	{
		shape.move(sf::Vector2f(-bounds.position.x, 0.f));
		if (velocity != nullptr)
		{
			velocity->x = std::abs(velocity->x);
		}
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, 0.f));
		if (velocity != nullptr)
		{
			velocity->x = -std::abs(velocity->x);
		}
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < 0.f)
	{
		shape.move(sf::Vector2f(0.f, -bounds.position.y));
		if (velocity != nullptr)
		{
			velocity->y = std::abs(velocity->y);
		}
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(0.f, offset));
		if (velocity != nullptr)
		{
			velocity->y = -std::abs(velocity->y);
		}
	}
}

sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window)
{
	const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
	return window.mapPixelToCoords(mousePixel);
}

void UpdatePlayerShipAim(sf::ConvexShape& ship, const sf::RenderWindow& window)
{
	const sf::Vector2f shipCenter = ship.getPosition();
	const sf::Vector2f mousePosition = GetMouseWorldPosition(window);
	RotateShipTowardPoint(ship, shipCenter, mousePosition);
}

void UpdateEnemies(
	std::vector<EnemyShip>& enemies,
	std::vector<Bullet>& bullets,
	const std::vector<OrbitSatellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	const GameContext& context)
{
	const float moveSpeed = enemyMoveSpeed * context.scale;
	const float arrivalDistance = 4.f * context.scale;

	for (auto& enemy : enemies)
	{
		const sf::Vector2f currentPosition = enemy.shape.getPosition();
		sf::Vector2f toTarget = enemy.targetPosition - currentPosition;
		const float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

		if (distance > arrivalDistance)
		{
			toTarget /= distance;
			enemy.shape.move(toTarget * moveSpeed * deltaTime);
			KeepInsideScreen(enemy.shape, context, nullptr);
			RotateShipTowardPoint(enemy.shape, currentPosition, earthCenter);
			continue;
		}

		const std::optional<sf::Vector2f> satelliteTarget = FindNearestSatellitePosition(currentPosition, satellites);
		if (!satelliteTarget.has_value())
		{
			RotateShipTowardPoint(enemy.shape, currentPosition, earthCenter);
			continue;
		}

		RotateShipTowardPoint(enemy.shape, currentPosition, *satelliteTarget);

		enemy.fireCooldown -= deltaTime;
		if (enemy.fireCooldown > 0.f)
		{
			continue;
		}

		bullets.push_back(CreateBulletFromShip(enemy.shape, context));
		enemy.fireCooldown = enemyFireInterval;
	}
}

void UpdateAsteroids(
	std::vector<Asteroid>& asteroids,
	float deltaTime,
	const GameContext& context,
	std::mt19937& rng)
{
	for (auto& asteroid : asteroids)
	{
		asteroid.shape.move(asteroid.velocity * deltaTime);
		KeepInsideScreen(asteroid.shape, context, &asteroid.velocity);
	}

	while (static_cast<int>(asteroids.size()) < targetAsteroidCount)
	{
		asteroids.push_back(CreateRandomAsteroid(context, rng));
	}
}

void UpdateNeutralShips(std::vector<NeutralShip>& neutrals, float deltaTime, const GameContext& context, std::mt19937& rng)
{
	const float removeMargin = 80.f * context.scale;

	for (auto& neutral : neutrals)
	{
		neutral.shape.move(neutral.velocity * deltaTime);
	}

	neutrals.erase(
		std::remove_if(
			neutrals.begin(),
			neutrals.end(),
			[&](const NeutralShip& neutral) {
				return IsFullyOutsideScreen(neutral.shape.getGlobalBounds(), context, removeMargin);
			}),
		neutrals.end());

	while (static_cast<int>(neutrals.size()) < neutralShipCount)
	{
		neutrals.push_back(CreateRandomNeutralShip(context, rng));
	}
}

void UpdateBullets(std::vector<Bullet>& bullets, float deltaTime, const GameContext& context)
{
	const float removeMargin = 40.f * context.scale;

	for (auto& bullet : bullets)
	{
		bullet.shape.move(bullet.velocity * deltaTime);
	}

	bullets.erase(
		std::remove_if(
			bullets.begin(),
			bullets.end(),
			[&](const Bullet& bullet) {
				const sf::Vector2f position = bullet.shape.getPosition();
				return position.x < -removeMargin
					|| position.x > context.windowWidth + removeMargin
					|| position.y < -removeMargin
					|| position.y > context.windowHeight + removeMargin;
			}),
		bullets.end());
}
} 

int main()
{
	const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
	const GameContext context = CreateGameContext(desktopMode);

	sf::RenderWindow window(desktopMode, "GAME", sf::State::Fullscreen);
	window.setFramerateLimit(60);
	(void)window.setActive(true);
	window.requestFocus();

	const sf::Vector2f earthCenter = GetEarthCenter(context);

	sf::CircleShape earth = CreateEarth(context);
	std::vector<OrbitSatellite> orbitSatellites = CreateOrbitSatellites(context);
	sf::ConvexShape playerShip = CreatePlayerShip(context);
	playerShip.setPosition(sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f));

	std::mt19937 rng(std::random_device{}());
	std::vector<Asteroid> asteroids = CreateInitialAsteroids(context, rng);
	std::vector<NeutralShip> neutralShips = CreateNeutralShips(context, rng);
	std::vector<EnemyShip> enemies;
	enemies.reserve(32);
	std::vector<Bullet> bullets;
	bullets.reserve(64);

	sf::Clock clock;

	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;

	while (window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		deltaTime = std::clamp(deltaTime, 1.f / 500.f, 0.05f);

		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
			else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
			{
				window.requestFocus();
				if (mousePressed->button == sf::Mouse::Button::Left)
				{
					UpdatePlayerShipAim(playerShip, window);
					bullets.push_back(CreateBulletFromShip(playerShip, context));
				}
			}
			else if (event->is<sf::Event::FocusGained>())
			{
				window.requestFocus();
			}
			else if (event->is<sf::Event::FocusLost>())
			{
				moveLeft = false;
				moveRight = false;
				moveUp = false;
				moveDown = false;
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				if (keyPressed->code == sf::Keyboard::Key::Escape)
				{
					window.close();
				}
				switch (keyPressed->scancode)
				{
				case sf::Keyboard::Scancode::A:
					moveLeft = true;
					break;
				case sf::Keyboard::Scancode::D:
					moveRight = true;
					break;
				case sf::Keyboard::Scancode::W:
					moveUp = true;
					break;
				case sf::Keyboard::Scancode::S:
					moveDown = true;
					break;
				default:
					break;
				}
			}
			else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
			{
				switch (keyReleased->scancode)
				{
				case sf::Keyboard::Scancode::A:
					moveLeft = false;
					break;
				case sf::Keyboard::Scancode::D:
					moveRight = false;
					break;
				case sf::Keyboard::Scancode::W:
					moveUp = false;
					break;
				case sf::Keyboard::Scancode::S:
					moveDown = false;
					break;
				default:
					break;
				}
			}
		}

		sf::Vector2f offset;
		if (moveLeft || IsMoveKeyPressed(sf::Keyboard::Scancode::A))
		{
			offset.x -= context.moveSpeed * deltaTime;
		}
		if (moveRight || IsMoveKeyPressed(sf::Keyboard::Scancode::D))
		{
			offset.x += context.moveSpeed * deltaTime;
		}
		if (moveUp || IsMoveKeyPressed(sf::Keyboard::Scancode::W))
		{
			offset.y -= context.moveSpeed * deltaTime;
		}
		if (moveDown || IsMoveKeyPressed(sf::Keyboard::Scancode::S))
		{
			offset.y += context.moveSpeed * deltaTime;
		}

		playerShip.move(offset);
		KeepInsideScreen(playerShip, context, nullptr);
		UpdatePlayerShipAim(playerShip, window);

		UpdateAsteroids(asteroids, deltaTime, context, rng);
		ProcessAsteroidMerges(asteroids);
		UpdateNeutralShips(neutralShips, deltaTime, context, rng);
		UpdateOrbitSatellites(orbitSatellites, deltaTime, earthCenter, context);
		UpdateEnemies(enemies, bullets, orbitSatellites, deltaTime, earthCenter, context);
		ProcessEnemyEnemyCollisions(enemies, context);
		ProcessAsteroidSatelliteCollisions(asteroids, orbitSatellites);
		ProcessAsteroidEnemyCollisions(asteroids, enemies, context);
		ProcessAsteroidNeutralCollisions(asteroids, neutralShips, context);
		UpdateBullets(bullets, deltaTime, context);
		ProcessBulletSatelliteCollisions(bullets, orbitSatellites);
		ProcessBulletEnemyCollisions(bullets, enemies, context);
		ProcessBulletAsteroidCollisions(bullets, asteroids, context);
		ProcessBulletNeutralCollisions(bullets, neutralShips, enemies, context, earthCenter, rng);
		if (ProcessEnemyPlayerCollision(enemies, playerShip, context))
		{
			window.close();
		}
		else if (ProcessAsteroidPlayerCollision(asteroids, playerShip, context))
		{
			window.close();
		}

		window.clear(sf::Color(10, 10, 25));
		window.draw(earth);
		for (const auto& satellite : orbitSatellites)
		{
			window.draw(satellite.shape);
			DrawSatelliteHpBar(window, satellite, context);
		}
		for (const auto& asteroid : asteroids)
		{
			window.draw(asteroid.shape);
		}
		for (const auto& neutral : neutralShips)
		{
			window.draw(neutral.shape);
		}
		for (const auto& enemy : enemies)
		{
			window.draw(enemy.shape);
		}
		for (const auto& bullet : bullets)
		{
			window.draw(bullet.shape);
		}
		window.draw(playerShip);
		window.display();
	}

	return 0;
}
