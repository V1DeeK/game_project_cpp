#include "collision/CollisionSystem.hpp"

#include "collision/Geometry.hpp"
#include "systems/SpawnSystem.hpp"

namespace
{
bool ProcessAsteroidPlayerCollision(
	std::vector<game::Asteroid>& asteroids,
	const game::PlayerShip& playerShip,
	const game::GameContext& context)
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

void ProcessBulletNeutralCollisions(
	std::vector<game::Bullet>& bullets,
	std::vector<game::NeutralShip>& neutrals,
	std::vector<game::EnemyShip>& enemies,
	const game::GameContext& context,
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
			game::SpawnSystem::SpawnEnemiesFromAllSides(enemies, context, earthCenter, rng);
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

bool ProcessPlayerCollisions(
	const game::PlayerShip& playerShip,
	std::vector<game::Asteroid>& asteroids,
	std::vector<game::EnemyShip>& enemies,
	std::vector<game::NeutralShip>& neutrals,
	std::vector<game::Satellite>& satellites,
	std::vector<game::Bullet>& bullets,
	const game::GameContext& context)
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
		if (!enemies[enemyIndex].IsOnScreen(context))
		{
			++enemyIndex;
			continue;
		}

		if (!enemies[enemyIndex].IntersectsRect(playerBounds))
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
	std::vector<game::Asteroid>& asteroids,
	std::vector<game::NeutralShip>& neutrals,
	const game::GameContext& context)
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
	std::vector<game::Asteroid>& asteroids,
	std::vector<game::EnemyShip>& enemies,
	const game::GameContext& context)
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
			if (!enemies[enemyIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!asteroids[asteroidIndex].IntersectsRect(enemies[enemyIndex].GetBounds()))
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

void ProcessAsteroidSatelliteCollisions(std::vector<game::Asteroid>& asteroids, std::vector<game::Satellite>& satellites)
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
	std::vector<game::EnemyShip>& enemies,
	std::vector<game::NeutralShip>& neutrals,
	const game::GameContext& context)
{
	for (std::size_t enemyIndex = 0; enemyIndex < enemies.size();)
	{
		bool enemyRemoved = false;

		for (std::size_t neutralIndex = 0; neutralIndex < neutrals.size(); ++neutralIndex)
		{
			if (!enemies[enemyIndex].IsOnScreen(context) || !neutrals[neutralIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!enemies[enemyIndex].IntersectsRect(neutrals[neutralIndex].GetBounds()))
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
	std::vector<game::EnemyShip>& enemies,
	std::vector<game::Satellite>& satellites,
	const game::GameContext& context)
{
	for (std::size_t enemyIndex = 0; enemyIndex < enemies.size();)
	{
		bool enemyRemoved = false;
		const sf::FloatRect enemyBounds = enemies[enemyIndex].GetBounds();

		if (!enemies[enemyIndex].IsOnScreen(context))
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

void ProcessEnemyEnemyCollisions(std::vector<game::EnemyShip>& enemies, const game::GameContext& context)
{
	for (std::size_t firstIndex = 0; firstIndex < enemies.size();)
	{
		bool pairRemoved = false;

		for (std::size_t secondIndex = firstIndex + 1; secondIndex < enemies.size(); ++secondIndex)
		{
			if (!enemies[firstIndex].IsOnScreen(context) || !enemies[secondIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!game::RectsIntersect(enemies[firstIndex].GetBounds(), enemies[secondIndex].GetBounds()))
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

void ProcessBulletBulletCollisions(std::vector<game::Bullet>& bullets)
{
	for (std::size_t firstIndex = 0; firstIndex < bullets.size();)
	{
		bool pairRemoved = false;

		for (std::size_t secondIndex = firstIndex + 1; secondIndex < bullets.size(); ++secondIndex)
		{
			const sf::FloatRect firstBounds = bullets[firstIndex].GetBounds();
			const sf::FloatRect secondBounds = bullets[secondIndex].GetBounds();
			if (!game::RectsIntersect(firstBounds, secondBounds))
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
	std::vector<game::Bullet>& bullets,
	std::vector<game::EnemyShip>& enemies,
	const game::GameContext& context)
{
	for (std::size_t bulletIndex = 0; bulletIndex < bullets.size();)
	{
		bool bulletHit = false;

		for (std::size_t enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex)
		{
			if (!enemies[enemyIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!bullets[bulletIndex].IntersectsConvexShape(enemies[enemyIndex].GetShape()))
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

void ProcessBulletSatelliteCollisions(std::vector<game::Bullet>& bullets, std::vector<game::Satellite>& satellites)
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
	std::vector<game::Bullet>& bullets,
	std::vector<game::Asteroid>& asteroids,
	const game::GameContext& context)
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
} // namespace

namespace game
{
bool CollisionSystem::ProcessFrame(
	const PlayerShip& playerShip,
	std::vector<Asteroid>& asteroids,
	std::vector<EnemyShip>& enemies,
	std::vector<NeutralShip>& neutrals,
	std::vector<Satellite>& satellites,
	std::vector<Bullet>& bullets,
	const GameContext& context,
	sf::Vector2f earthCenter,
	std::mt19937& rng)
{
	ProcessEnemyEnemyCollisions(enemies, context);
	ProcessEnemyNeutralCollisions(enemies, neutrals, context);
	ProcessEnemySatelliteCollisions(enemies, satellites, context);
	ProcessAsteroidSatelliteCollisions(asteroids, satellites);
	ProcessAsteroidEnemyCollisions(asteroids, enemies, context);
	ProcessAsteroidNeutralCollisions(asteroids, neutrals, context);

	ProcessBulletBulletCollisions(bullets);
	ProcessBulletSatelliteCollisions(bullets, satellites);
	ProcessBulletEnemyCollisions(bullets, enemies, context);
	ProcessBulletAsteroidCollisions(bullets, asteroids, context);
	ProcessBulletNeutralCollisions(bullets, neutrals, enemies, context, earthCenter, rng);

	return ProcessPlayerCollisions(playerShip, asteroids, enemies, neutrals, satellites, bullets, context);
}
} // namespace game
