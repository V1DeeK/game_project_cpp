#include "entities/EnemyShip.hpp"

#include <utility>

#include "collision/Geometry.hpp"
#include "config/GameConstants.hpp"

namespace game
{
EnemyShip::EnemyShip(Ship ship)
	: Ship(std::move(ship))
{
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

	if (bounds.position.x < screenOrigin)
	{
		shape.move(sf::Vector2f(-bounds.position.x, screenOrigin));
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, screenOrigin));
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < screenOrigin)
	{
		shape.move(sf::Vector2f(screenOrigin, -bounds.position.y));
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(screenOrigin, offset));
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
