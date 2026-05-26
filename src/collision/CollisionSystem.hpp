#pragma once

#include <random>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "entities/Asteroid.hpp"
#include "entities/Bullet.hpp"
#include "entities/EnemyShip.hpp"
#include "entities/NeutralShip.hpp"
#include "entities/PlayerShip.hpp"
#include "entities/Satellite.hpp"

namespace game
{
class CollisionSystem
{
public:
	static bool ProcessFrame(
		const PlayerShip& playerShip,
		std::vector<Asteroid>& asteroids,
		std::vector<EnemyShip>& enemies,
		std::vector<NeutralShip>& neutrals,
		std::vector<Satellite>& satellites,
		std::vector<Bullet>& bullets,
		const GameContext& context,
		sf::Vector2f earthCenter,
		std::mt19937& rng);
};
} // namespace game
