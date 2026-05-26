#pragma once

#include <vector>

#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "entities/Bullet.hpp"
#include "entities/EnemyShip.hpp"
#include "entities/Satellite.hpp"

namespace game
{
class EnemyAISystem
{
public:
	static void UpdateAll(
		std::vector<EnemyShip>& enemies,
		std::vector<Bullet>& bullets,
		const std::vector<Satellite>& satellites,
		float deltaTime,
		sf::Vector2f earthCenter,
		sf::Vector2f playerPosition,
		const GameContext& context);
};
} // namespace game
