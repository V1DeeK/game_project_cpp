#include "entities/Earth.hpp"

#include <utility>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"

namespace game
{
Earth::Earth(sf::CircleShape shape)
	: m_shape(std::move(shape))
{
}

Earth Earth::Create(const GameContext& context)
{
	sf::CircleShape shape(context.earthRadius);
	shape.setOrigin(sf::Vector2f(context.earthRadius, context.earthRadius));
	shape.setPosition(GetEarthCenter(context));
	shape.setFillColor(kColorEarthFill);
	shape.setOutlineColor(kColorEarthOutline);
	shape.setOutlineThickness(earthOutlineThickness);
	return Earth(std::move(shape));
}

void Earth::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

sf::Vector2f Earth::GetCenter() const
{
	return m_shape.getPosition();
}
} // namespace game
