#include "entities/Ship.hpp"

#include <cmath>
#include <utility>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"

namespace
{
sf::ConvexShape CreateTriangleShip(const game::GameContext& context, sf::Color fillColor, sf::Color outlineColor)
{
	sf::ConvexShape ship(game::shipTriangleVertexCount);
	ship.setPoint(
		game::triangleVertexNose,
		sf::Vector2f(context.shipWidth * game::half, game::screenOrigin));
	ship.setPoint(game::triangleVertexBottomLeft, sf::Vector2f(game::screenOrigin, context.shipHeight));
	ship.setPoint(game::triangleVertexBottomRight, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth * game::half, context.shipHeight * game::half));
	ship.setFillColor(fillColor);
	ship.setOutlineColor(outlineColor);
	ship.setOutlineThickness(game::defaultOutlineThickness);
	return ship;
}
} // namespace

namespace game
{
Ship::Ship(sf::ConvexShape shape)
	: m_shape(std::move(shape))
{
}

Ship Ship::CreateEnemyShip(const GameContext& context)
{
	return Ship(CreateTriangleShip(context, kColorEnemyFill, kColorEnemyOutline));
}

void Ship::RotateToward(sf::ConvexShape& shape, sf::Vector2f shipCenter, sf::Vector2f targetPoint)
{
	sf::Vector2f direction = targetPoint - shipCenter;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= directionEpsilon)
	{
		return;
	}

	const float angleDegrees = std::atan2(direction.y, direction.x) * degreesPerRadian + shipRotationOffsetDegrees;
	shape.setRotation(sf::degrees(angleDegrees));
}

sf::Vector2f Ship::GetForwardDirection(const sf::ConvexShape& shape, const GameContext& context)
{
	const sf::Transform transform = shape.getTransform();
	const sf::Vector2f nose = transform.transformPoint(sf::Vector2f(context.shipWidth * half, screenOrigin));
	const sf::Vector2f body = transform.transformPoint(
		sf::Vector2f(context.shipWidth * half, context.shipHeight * shipBodyForwardFactor));
	sf::Vector2f direction = nose - body;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > directionEpsilon)
	{
		direction /= length;
	}
	return direction;
}

sf::Vector2f Ship::GetNosePosition(const sf::ConvexShape& shape, const GameContext& context)
{
	return shape.getTransform().transformPoint(sf::Vector2f(context.shipWidth * half, screenOrigin));
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
