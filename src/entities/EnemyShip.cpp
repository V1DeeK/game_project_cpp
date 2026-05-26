#include "entities/EnemyShip.hpp"

#include <cmath>
#include <random>
#include <utility>

#include "collision/Geometry.hpp"
#include "config/GameConstants.hpp"
#include "systems/FiringRing.hpp"

namespace game
{
EnemyShip::EnemyShip(Ship ship)
	: Ship(std::move(ship))
{
}

void EnemyShip::SpawnFromAllSides(
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

		const FiringSlotAssignment slot = FiringRing::FindNearestSlot(spawnPosition, earthCenter, context, enemies);

		std::uniform_int_distribution<int> behaviorDist(0, 1);

		EnemyShip enemy(Ship::CreateEnemyShip(context));
		enemy.SetPosition(spawnPosition);
		enemy.m_targetRingIndex = slot.ringIndex;
		enemy.m_targetPointIndex = slot.pointIndex;
		enemy.m_targetPosition = FiringRing::GetPointPosition(earthCenter, context, slot.ringIndex, slot.pointIndex);
		enemy.m_behavior = behaviorDist(rng) == 0 ? EnemyBehavior::AttackSatellites : EnemyBehavior::HuntPlayer;
		enemy.RotateTowardFromPosition(spawnPosition, earthCenter);
		enemies.push_back(enemy);
	}
}

sf::Vector2f EnemyShip::CreateSpawnPositionFromEdge(int edge, const GameContext& context, float spawnMargin, std::mt19937& rng)
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

bool EnemyShip::IsOnScreen(const GameContext& context) const
{
	return IsShapeOnScreen(GetShape(), context);
}

bool EnemyShip::IntersectsRect(const sf::FloatRect& rect) const
{
	return RectsIntersect(GetBounds(), rect);
}

void EnemyShip::ClampToScreen(const GameContext& context)
{
	sf::Shape& shape = GetShape();
	sf::FloatRect bounds = shape.getGlobalBounds();

	if (bounds.position.x < 0.f)
	{
		shape.move(sf::Vector2f(-bounds.position.x, 0.f));
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, 0.f));
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < 0.f)
	{
		shape.move(sf::Vector2f(0.f, -bounds.position.y));
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(0.f, offset));
	}
}

void EnemyShip::RotateTowardFromPosition(sf::Vector2f fromPosition, sf::Vector2f targetPoint)
{
	Ship::RotateToward(GetShape(), fromPosition, targetPoint);
}

EnemyBehavior EnemyShip::GetBehavior() const
{
	return m_behavior;
}

sf::Vector2f EnemyShip::GetTargetPosition() const
{
	return m_targetPosition;
}

int EnemyShip::GetTargetRingIndex() const
{
	return m_targetRingIndex;
}

int EnemyShip::GetTargetPointIndex() const
{
	return m_targetPointIndex;
}

float EnemyShip::GetFireCooldown() const
{
	return m_fireCooldown;
}

void EnemyShip::SetFireCooldown(float cooldown)
{
	m_fireCooldown = cooldown;
}

void EnemyShip::TickFireCooldown(float deltaTime)
{
	m_fireCooldown -= deltaTime;
}
} // namespace game
