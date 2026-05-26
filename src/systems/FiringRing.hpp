#pragma once

#include <vector>

#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "config/GameConstants.hpp"

namespace game
{
class EnemyShip;

struct FiringSlotAssignment
{
	int ringIndex = initialTargetRingIndex;
	int pointIndex = initialTargetPointIndex;
};

class FiringRing
{
public:
	static sf::Vector2f GetPointPosition(
		sf::Vector2f earthCenter,
		const GameContext& context,
		int ringIndex,
		int pointIndex);

	static FiringSlotAssignment FindNearestSlot(
		sf::Vector2f spawnPosition,
		sf::Vector2f earthCenter,
		const GameContext& context,
		const std::vector<EnemyShip>& enemies);
};
} // namespace game
