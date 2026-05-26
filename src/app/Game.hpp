#pragma once

#include <random>
#include <vector>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include "app/GameContext.hpp"
#include "entities/Asteroid.hpp"
#include "entities/Bullet.hpp"
#include "entities/Earth.hpp"
#include "entities/EnemyShip.hpp"
#include "entities/NeutralShip.hpp"
#include "entities/PlayerShip.hpp"
#include "entities/Satellite.hpp"
#include "input/PlayerInput.hpp"

namespace game
{
class Game
{
public:
	Game();
	void Run();

private:
	void ProcessEvents();
	void Update(float deltaTime);
	void Render();

	sf::RenderWindow m_window;
	GameContext m_context;
	sf::Clock m_clock;

	Earth m_earth;
	sf::Vector2f m_earthCenter;
	std::vector<Satellite> m_orbitSatellites;
	PlayerShip m_playerShip;
	PlayerInput m_playerInput;

	std::mt19937 m_rng;
	std::vector<Asteroid> m_asteroids;
	std::vector<NeutralShip> m_neutralShips;
	std::vector<EnemyShip> m_enemies;
	std::vector<Bullet> m_bullets;
};
} // namespace game
