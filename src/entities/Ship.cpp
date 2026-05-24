#include "entities/Ship.hpp"

#include <cmath>
#include <utility>

#include "config/Colors.hpp"

namespace
{
sf::ConvexShape CreateTriangleShip(const game::GameContext& context, sf::Color fillColor, sf::Color outlineColor)
{
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight / 2.f));
	ship.setFillColor(fillColor);
	ship.setOutlineColor(outlineColor);
	ship.setOutlineThickness(1.f);
	return ship;
}
} // namespace

namespace game
{
Ship::Ship(sf::ConvexShape shape)
	: m_shape(std::move(shape))
{
}

Ship Ship::CreatePlayerShip(const GameContext& context)
{
	return Ship(CreateTriangleShip(context, kColorPlayerShipFill, kColorPlayerShipOutline));
}

Ship Ship::CreateEnemyShip(const GameContext& context)
{
	return Ship(CreateTriangleShip(context, kColorEnemyFill, kColorEnemyOutline));
}

void Ship::RotateToward(sf::ConvexShape& shape, sf::Vector2f shipCenter, sf::Vector2f targetPoint)
{
	sf::Vector2f direction = targetPoint - shipCenter;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= 0.0001f)
	{
		return;
	}

	const float angleDegrees = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f + 90.f;
	shape.setRotation(sf::degrees(angleDegrees));
}

sf::Vector2f Ship::GetForwardDirection(const sf::ConvexShape& shape, const GameContext& context)
{
	const sf::Transform transform = shape.getTransform();
	const sf::Vector2f nose = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
	const sf::Vector2f body = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight * 0.5f));
	sf::Vector2f direction = nose - body;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.0001f)
	{
		direction /= length;
	}
	return direction;
}

sf::Vector2f Ship::GetNosePosition(const sf::ConvexShape& shape, const GameContext& context)
{
	return shape.getTransform().transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
}

void Ship::RotateToward(sf::Vector2f targetPoint)
{
	RotateToward(m_shape, m_shape.getPosition(), targetPoint);
}

void Ship::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

sf::Vector2f Ship::GetPosition() const
{
	return m_shape.getPosition();
}

void Ship::SetPosition(sf::Vector2f position)
{
	m_shape.setPosition(position);
}

void Ship::Move(sf::Vector2f offset)
{
	m_shape.move(offset);
}

sf::FloatRect Ship::GetBounds() const
{
	return m_shape.getGlobalBounds();
}

sf::ConvexShape& Ship::GetShape()
{
	return m_shape;
}

const sf::ConvexShape& Ship::GetShape() const
{
	return m_shape;
}
} // namespace game
