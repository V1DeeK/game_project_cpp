#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "app/GameContext.hpp"
#include "collision/Geometry.hpp"
#include "entities/Asteroid.hpp"
#include "entities/Bullet.hpp"
#include "entities/Earth.hpp"
#include "entities/Satellite.hpp"
#include "entities/NeutralShip.hpp"
#include "entities/PlayerShip.hpp"
#include "entities/Ship.hpp"
#include "systems/FiringSystem.hpp"
#include "systems/SatelliteOrbitSystem.hpp"

using game::Asteroid;
using game::Bullet;
using game::CreateGameContext;
using game::GameContext;
using game::FiringSystem;
using game::SatelliteOrbitSystem;
using game::Earth;
using game::Satellite;
using game::NeutralShip;
using game::PlayerShip;

enum class EnemyBehavior
{
	AttackSatellites,
	HuntPlayer,
};

struct EnemyShip
{
	sf::ConvexShape shape;
	sf::Vector2f targetPosition;
	int targetRingIndex = 0;
	int targetPointIndex = 0;
	float fireCooldown = 0.f;
	EnemyBehavior behavior = EnemyBehavior::AttackSatellites;
};

namespace
{
using namespace game;

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

		std::uniform_int_distribution<int> behaviorDist(0, 1);

		EnemyShip enemy;
		enemy.shape = Ship::CreateEnemyShip(context).GetShape();
		enemy.shape.setPosition(spawnPosition);
		enemy.targetRingIndex = slot.ringIndex;
		enemy.targetPointIndex = slot.pointIndex;
		enemy.targetPosition = GetFiringPointPosition(earthCenter, context, slot.ringIndex, slot.pointIndex);
		enemy.behavior = behaviorDist(rng) == 0 ? EnemyBehavior::AttackSatellites : EnemyBehavior::HuntPlayer;
		Ship::RotateToward(enemy.shape, spawnPosition, earthCenter);
		enemies.push_back(enemy);
	}
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
			if (!neutrals[neutralIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!bullets[bulletIndex].IntersectsConvexShape(neutrals[neutralIndex].GetShape()))
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

bool ProcessAsteroidPlayerCollision(
	std::vector<Asteroid>& asteroids,
	const PlayerShip& playerShip,
	const GameContext& context)
{
	const sf::FloatRect playerBounds = playerShip.GetBounds();

	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size(); ++asteroidIndex)
	{
		if (!asteroids[asteroidIndex].IsOnScreen(context))
		{
			continue;
		}

		if (asteroids[asteroidIndex].IntersectsRect(playerBounds))
		{
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
			return true;
		}
	}

	return false;
}

bool ProcessPlayerCollisions(
	const PlayerShip& playerShip,
	std::vector<Asteroid>& asteroids,
	std::vector<EnemyShip>& enemies,
	std::vector<NeutralShip>& neutrals,
	std::vector<Satellite>& satellites,
	std::vector<Bullet>& bullets,
	const GameContext& context)
{
	const sf::FloatRect playerBounds = playerShip.GetBounds();

	for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
	{
		if (!satellites[satelliteIndex].IntersectsRect(playerBounds))
		{
			continue;
		}

		if (satellites[satelliteIndex].TakeDamage(1))
		{
			satellites.erase(satellites.begin() + static_cast<std::ptrdiff_t>(satelliteIndex));
		}
		return false;
	}

	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size(); ++bulletIndex)
	{
		if (!bullets[bulletIndex].IntersectsConvexShape(playerShip.GetShape()))
		{
			continue;
		}

		bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
		return true;
	}

	if (ProcessAsteroidPlayerCollision(asteroids, playerShip, context))
	{
		return true;
	}

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

	for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size();)
	{
		if (!neutrals[neutralIndex].IsOnScreen(context))
		{
			++neutralIndex;
			continue;
		}

		if (!neutrals[neutralIndex].IntersectsRect(playerBounds))
		{
			++neutralIndex;
			continue;
		}

		neutrals.erase(neutrals.begin() + static_cast<std::ptrdiff_t>(neutralIndex));
		return true;
	}

	return false;
}

void ProcessAsteroidNeutralCollisions(
	std::vector<Asteroid>& asteroids,
	std::vector<NeutralShip>& neutrals,
	const GameContext& context)
{
	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size();)
	{
		bool asteroidDestroyed = false;

		if (!asteroids[asteroidIndex].IsOnScreen(context))
		{
			++asteroidIndex;
			continue;
		}

		for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size(); ++neutralIndex)
		{
			if (!neutrals[neutralIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!asteroids[asteroidIndex].IntersectsRect(neutrals[neutralIndex].GetBounds()))
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

		if (!asteroids[asteroidIndex].IsOnScreen(context))
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

			if (!asteroids[asteroidIndex].IntersectsRect(enemies[enemyIndex].shape.getGlobalBounds()))
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

void ProcessAsteroidSatelliteCollisions(std::vector<Asteroid>& asteroids, std::vector<Satellite>& satellites)
{
	for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size();)
	{
		bool asteroidDestroyed = false;

		for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
		{
			if (!satellites[satelliteIndex].IntersectsCircle(
					asteroids[asteroidIndex].GetCenter(),
					asteroids[asteroidIndex].GetCollisionRadius()))
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

void ProcessEnemyNeutralCollisions(
	std::vector<EnemyShip>& enemies,
	std::vector<NeutralShip>& neutrals,
	const GameContext& context)
{
	for (std::size_t enemyIndex = 0; enemyIndex < enemies.size();)
	{
		bool enemyRemoved = false;

		for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size(); ++neutralIndex)
		{
			if (!IsShapeOnScreen(enemies[enemyIndex].shape, context)
				|| !neutrals[neutralIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!RectsIntersect(enemies[enemyIndex].shape.getGlobalBounds(), neutrals[neutralIndex].GetBounds()))
			{
				continue;
			}

			neutrals.erase(neutrals.begin() + static_cast<std::ptrdiff_t>(neutralIndex));
			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
			enemyRemoved = true;
			break;
		}

		if (!enemyRemoved)
		{
			++enemyIndex;
		}
	}
}

void ProcessEnemySatelliteCollisions(
	std::vector<EnemyShip>& enemies,
	std::vector<Satellite>& satellites,
	const GameContext& context)
{
	for (std::size_t enemyIndex = 0; enemyIndex < enemies.size();)
	{
		bool enemyRemoved = false;
		const sf::FloatRect enemyBounds = enemies[enemyIndex].shape.getGlobalBounds();

		if (!IsShapeOnScreen(enemies[enemyIndex].shape, context))
		{
			++enemyIndex;
			continue;
		}

		for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
		{
			if (!satellites[satelliteIndex].IntersectsRect(enemyBounds))
			{
				continue;
			}

			satellites.erase(satellites.begin() + static_cast<std::ptrdiff_t>(satelliteIndex));
			enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
			enemyRemoved = true;
			break;
		}

		if (!enemyRemoved)
		{
			++enemyIndex;
		}
	}
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

void ProcessBulletBulletCollisions(std::vector<Bullet>& bullets)
{
	for (std::size_t firstIndex = 0; firstIndex < bullets.size();)
	{
		bool pairRemoved = false;

		for (std::size_t secondIndex = firstIndex + 1; secondIndex < bullets.size(); ++secondIndex)
		{
			const sf::FloatRect firstBounds = bullets[firstIndex].GetBounds();
			const sf::FloatRect secondBounds = bullets[secondIndex].GetBounds();
			if (!RectsIntersect(firstBounds, secondBounds))
			{
				continue;
			}

			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(secondIndex));
			bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(firstIndex));
			pairRemoved = true;
			break;
		}

		if (!pairRemoved)
		{
			++firstIndex;
		}
	}
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

			if (!bullets[bulletIndex].IntersectsConvexShape(enemies[enemyIndex].shape))
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

void ProcessBulletSatelliteCollisions(std::vector<Bullet>& bullets, std::vector<Satellite>& satellites)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		const sf::Vector2f bulletCenter = bullets[bulletIndex].GetCenter();

		bool bulletHit = false;

		for (std::size_t satelliteIndex = 0; satelliteIndex < satellites.size(); ++satelliteIndex)
		{
			if (!satellites[satelliteIndex].ContainsPoint(bulletCenter))
			{
				continue;
			}

			if (satellites[satelliteIndex].TakeDamage(1))
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
		const sf::Vector2f bulletCenter = bullets[bulletIndex].GetCenter();

		bool bulletHit = false;

		for (std::size_t asteroidIndex = 0; asteroidIndex < asteroids.size(); ++asteroidIndex)
		{
			if (!asteroids[asteroidIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!asteroids[asteroidIndex].ContainsPoint(bulletCenter))
			{
				continue;
			}

			if (asteroids[asteroidIndex].TakeDamage(1))
			{
				asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(asteroidIndex));
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

void UpdateEnemies(
	std::vector<EnemyShip>& enemies,
	std::vector<Bullet>& bullets,
	const std::vector<Satellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	sf::Vector2f playerPosition,
	const GameContext& context)
{
	const float moveSpeed = enemyMoveSpeed * context.scale;
	const float arrivalDistance = 4.f * context.scale;

	for (auto& enemy : enemies)
	{
		const sf::Vector2f currentPosition = enemy.shape.getPosition();

		if (enemy.behavior == EnemyBehavior::HuntPlayer)
		{
			sf::Vector2f toPlayer = playerPosition - currentPosition;
			const float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
			if (distance > arrivalDistance)
			{
				toPlayer /= distance;
				enemy.shape.move(toPlayer * moveSpeed * deltaTime);
				KeepInsideScreen(enemy.shape, context, nullptr);
			}

			Ship::RotateToward(enemy.shape, enemy.shape.getPosition(), playerPosition);

			enemy.fireCooldown -= deltaTime;
			if (enemy.fireCooldown <= 0.f)
			{
				bullets.push_back(FiringSystem::CreateBulletFromShip(enemy.shape, context));
				enemy.fireCooldown = enemyFireInterval;
			}
			continue;
		}

		sf::Vector2f toTarget = enemy.targetPosition - currentPosition;
		const float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

		if (distance > arrivalDistance)
		{
			toTarget /= distance;
			enemy.shape.move(toTarget * moveSpeed * deltaTime);
			KeepInsideScreen(enemy.shape, context, nullptr);
			Ship::RotateToward(enemy.shape, currentPosition, earthCenter);
			continue;
		}

		const std::optional<sf::Vector2f> satelliteTarget = Satellite::FindNearestPosition(currentPosition, satellites);
		if (!satelliteTarget.has_value())
		{
			Ship::RotateToward(enemy.shape, currentPosition, earthCenter);
			continue;
		}

		Ship::RotateToward(enemy.shape, currentPosition, *satelliteTarget);

		enemy.fireCooldown -= deltaTime;
		if (enemy.fireCooldown <= 0.f)
		{
			bullets.push_back(FiringSystem::CreateBulletFromShip(enemy.shape, context));
			enemy.fireCooldown = enemyFireInterval;
		}
	}
}

} // namespace

int main()
{
	const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
	const GameContext context = CreateGameContext(desktopMode);

	sf::RenderWindow window(desktopMode, "GAME", sf::State::Fullscreen);
	window.setFramerateLimit(60);
	(void)window.setActive(true);
	window.requestFocus();

	Earth earth = Earth::Create(context);
	const sf::Vector2f earthCenter = earth.GetCenter();
	std::vector<Satellite> orbitSatellites = SatelliteOrbitSystem::CreateOrbitSatellites(context);
	PlayerShip playerShip = PlayerShip::CreateAtCenter(context);

	std::mt19937 rng(std::random_device{}());
	std::vector<Asteroid> asteroids = Asteroid::CreateInitialFleet(context, rng);
	std::vector<NeutralShip> neutralShips = NeutralShip::CreateInitialFleet(context, rng);
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
					playerShip.UpdateAim(window);
					bullets.push_back(FiringSystem::CreateBulletFromShip(playerShip, context));
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

		playerShip.ApplyMovement(moveLeft, moveRight, moveUp, moveDown, deltaTime, context);
		playerShip.ClampToScreen(context);
		playerShip.UpdateAim(window);

		Asteroid::UpdateAll(asteroids, deltaTime, context, rng);
		Asteroid::ProcessMerges(asteroids);
		NeutralShip::UpdateAll(neutralShips, deltaTime, context, rng);
		SatelliteOrbitSystem::UpdateOrbitSatellites(orbitSatellites, deltaTime, earthCenter, context);
		UpdateEnemies(
			enemies,
			bullets,
			orbitSatellites,
			deltaTime,
			earthCenter,
			playerShip.GetPosition(),
			context);
		ProcessEnemyEnemyCollisions(enemies, context);
		ProcessEnemyNeutralCollisions(enemies, neutralShips, context);
		ProcessEnemySatelliteCollisions(enemies, orbitSatellites, context);
		ProcessAsteroidSatelliteCollisions(asteroids, orbitSatellites);
		ProcessAsteroidEnemyCollisions(asteroids, enemies, context);
		ProcessAsteroidNeutralCollisions(asteroids, neutralShips, context);
		Bullet::UpdateAll(bullets, deltaTime, context);
		ProcessBulletBulletCollisions(bullets);
		ProcessBulletSatelliteCollisions(bullets, orbitSatellites);
		ProcessBulletEnemyCollisions(bullets, enemies, context);
		ProcessBulletAsteroidCollisions(bullets, asteroids, context);
		ProcessBulletNeutralCollisions(bullets, neutralShips, enemies, context, earthCenter, rng);
		if (ProcessPlayerCollisions(
				playerShip,
				asteroids,
				enemies,
				neutralShips,
				orbitSatellites,
				bullets,
				context))
		{
			window.close();
		}

		window.clear(sf::Color(10, 10, 25));
		earth.Draw(window);
		for (const auto& satellite : orbitSatellites)
		{
			satellite.Draw(window);
			satellite.DrawHpBar(window, context);
		}
		for (const auto& asteroid : asteroids)
		{
			asteroid.Draw(window);
		}
		for (const auto& neutral : neutralShips)
		{
			neutral.Draw(window);
		}
		for (const auto& enemy : enemies)
		{
			window.draw(enemy.shape);
		}
		for (const auto& bullet : bullets)
		{
			bullet.Draw(window);
		}
		playerShip.Draw(window);
		window.display();
	}

	return 0;
}
