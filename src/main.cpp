#include <algorithm>
#include <cmath>
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
#include "entities/EnemyShip.hpp"
#include "entities/NeutralShip.hpp"
#include "entities/PlayerShip.hpp"
#include "entities/Ship.hpp"
#include "systems/FiringRing.hpp"
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
using game::EnemyShip;
using game::NeutralShip;
using game::PlayerShip;

namespace
{
using namespace game;

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
			EnemyShip::SpawnFromAllSides(enemies, context, earthCenter, rng);
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
			if (!enemies[enemyIndex].IsOnScreen(context)
				|| !neutrals[neutralIndex].IsOnScreen(context))
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
	std::vector<EnemyShip>& enemies,
	std::vector<Satellite>& satellites,
	const GameContext& context)
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

void ProcessEnemyEnemyCollisions(std::vector<EnemyShip>& enemies, const GameContext& context)
{
	for (std::size_t firstIndex = 0; firstIndex < enemies.size();)
	{
		bool pairRemoved = false;

		for (std::size_t secondIndex = firstIndex + 1; secondIndex < enemies.size(); ++secondIndex)
		{
			if (!enemies[firstIndex].IsOnScreen(context)
				|| !enemies[secondIndex].IsOnScreen(context))
			{
				continue;
			}

			if (!RectsIntersect(enemies[firstIndex].GetBounds(), enemies[secondIndex].GetBounds()))
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
		const sf::Vector2f currentPosition = enemy.GetPosition();

		if (enemy.GetBehavior() == EnemyBehavior::HuntPlayer)
		{
			sf::Vector2f toPlayer = playerPosition - currentPosition;
			const float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
			if (distance > arrivalDistance)
			{
				toPlayer /= distance;
				enemy.Move(toPlayer * moveSpeed * deltaTime);
				enemy.ClampToScreen(context);
			}

			enemy.RotateToward(playerPosition);

			enemy.TickFireCooldown(deltaTime);
			if (enemy.GetFireCooldown() <= 0.f)
			{
				bullets.push_back(FiringSystem::CreateBulletFromShip(enemy, context));
				enemy.SetFireCooldown(enemyFireInterval);
			}
			continue;
		}

		sf::Vector2f toTarget = enemy.GetTargetPosition() - currentPosition;
		const float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

		if (distance > arrivalDistance)
		{
			toTarget /= distance;
			enemy.Move(toTarget * moveSpeed * deltaTime);
			enemy.ClampToScreen(context);
			enemy.RotateTowardFromPosition(currentPosition, earthCenter);
			continue;
		}

		const std::optional<sf::Vector2f> satelliteTarget = Satellite::FindNearestPosition(currentPosition, satellites);
		if (!satelliteTarget.has_value())
		{
			enemy.RotateTowardFromPosition(currentPosition, earthCenter);
			continue;
		}

		enemy.RotateTowardFromPosition(currentPosition, *satelliteTarget);

		enemy.TickFireCooldown(deltaTime);
		if (enemy.GetFireCooldown() <= 0.f)
		{
			bullets.push_back(FiringSystem::CreateBulletFromShip(enemy, context));
			enemy.SetFireCooldown(enemyFireInterval);
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
			enemy.Draw(window);
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
