#pragma once

#include <random>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "entities/EnemyShip.hpp"

namespace game
{
class SpawnSystem
{
public:
	static void SpawnEnemiesFromAllSides(
		std::vector<EnemyShip>& enemies,
		const GameContext& context,
		sf::Vector2f earthCenter,
		std::mt19937& rng);
};
} // namespace game
