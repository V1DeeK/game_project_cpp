#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "app/GameContext.hpp"

namespace game
{
class Earth
{
public:
	static Earth Create(const GameContext& context);

	void Draw(sf::RenderWindow& window) const;
	sf::Vector2f GetCenter() const;

private:
	explicit Earth(sf::CircleShape shape);

	sf::CircleShape m_shape;
};
} // namespace game
