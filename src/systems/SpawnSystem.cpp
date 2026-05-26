#include "systems/SpawnSystem.hpp"

#include <algorithm>
#include <random>

#include "config/GameConstants.hpp"
#include "entities/Ship.hpp"
#include "systems/FiringRing.hpp"

namespace
{
sf::Vector2f CreateSpawnPositionFromEdge(int edge, const game::GameContext& context, float spawnMargin, std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(game::screenOrigin, context.windowWidth);
	std::uniform_real_distribution<float> positionY(game::screenOrigin, context.windowHeight);

	switch (edge)
	{
	case game::screenEdgeTop:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case game::screenEdgeRight:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case game::screenEdgeBottom:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}
} // namespace

namespace game
{
void SpawnSystem::SpawnEnemiesFromAllSides(
	std::vector<EnemyShip>& enemies,
	const GameContext& context,
	sf::Vector2f earthCenter,
	std::mt19937& rng)
{
	const float spawnMargin = std::max(context.shipWidth, context.shipHeight) + offScreenSpawnMarginExtra * context.scale;

	for (int enemyIndex = hitPointsDepleted; enemyIndex < enemySpawnCount; ++enemyIndex)
	{
		const int edge = enemyIndex;
		const sf::Vector2f spawnPosition = CreateSpawnPositionFromEdge(edge, context, spawnMargin, rng);

		const FiringSlotAssignment slot = FiringRing::FindNearestSlot(spawnPosition, earthCenter, context, enemies);

		std::uniform_int_distribution<int> behaviorDist(enemyBehaviorRandomMin, enemyBehaviorRandomMax);

		EnemyShip enemy(Ship::CreateEnemyShip(context));
		enemy.SetPosition(spawnPosition);
		enemy.m_targetRingIndex = slot.ringIndex;
		enemy.m_targetPointIndex = slot.pointIndex;
		enemy.m_targetPosition = FiringRing::GetPointPosition(earthCenter, context, slot.ringIndex, slot.pointIndex);
		enemy.m_behavior = behaviorDist(rng) == enemyBehaviorAttackSatellitesRoll
			? EnemyBehavior::AttackSatellites
			: EnemyBehavior::HuntPlayer;
		enemy.RotateTowardFromPosition(spawnPosition, earthCenter);
		enemies.push_back(enemy);
	}
}
} // namespace game
