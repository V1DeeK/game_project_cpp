#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "app/GameContext.hpp"

namespace game
{
class Ship
{
public:
	Ship() = default;
	explicit Ship(sf::ConvexShape shape);

	static Ship CreatePlayerShip(const GameContext& context);
	static Ship CreateEnemyShip(const GameContext& context);

	static void RotateToward(sf::ConvexShape& shape, sf::Vector2f shipCenter, sf::Vector2f targetPoint);
	static sf::Vector2f GetForwardDirection(const sf::ConvexShape& shape, const GameContext& context);
	static sf::Vector2f GetNosePosition(const sf::ConvexShape& shape, const GameContext& context);

	void RotateToward(sf::Vector2f targetPoint);
	void Draw(sf::RenderWindow& window) const;

	sf::Vector2f GetPosition() const;
	void SetPosition(sf::Vector2f position);
	void Move(sf::Vector2f offset);

	sf::FloatRect GetBounds() const;

	sf::ConvexShape& GetShape();
	const sf::ConvexShape& GetShape() const;

private:
	sf::ConvexShape m_shape;
};
} // namespace game
