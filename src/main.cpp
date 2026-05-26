#include <algorithm>
#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

#include "app/GameContext.hpp"
#include "collision/CollisionSystem.hpp"
#include "entities/Asteroid.hpp"
#include "entities/Bullet.hpp"
#include "entities/Earth.hpp"
#include "entities/EnemyShip.hpp"
#include "entities/NeutralShip.hpp"
#include "entities/PlayerShip.hpp"
#include "entities/Satellite.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/FiringSystem.hpp"
#include "systems/SatelliteOrbitSystem.hpp"

using game::Asteroid;
using game::Bullet;
using game::CollisionSystem;
using game::CreateGameContext;
using game::Earth;
using game::EnemyAISystem;
using game::EnemyShip;
using game::FiringSystem;
using game::GameContext;
using game::NeutralShip;
using game::PlayerShip;
using game::Satellite;
using game::SatelliteOrbitSystem;

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
		EnemyAISystem::UpdateAll(
			enemies,
			bullets,
			orbitSatellites,
			deltaTime,
			earthCenter,
			playerShip.GetPosition(),
			context);
		Bullet::UpdateAll(bullets, deltaTime, context);
		if (CollisionSystem::ProcessFrame(
				playerShip,
				asteroids,
				enemies,
				neutralShips,
				orbitSatellites,
				bullets,
				context,
				earthCenter,
				rng))
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
