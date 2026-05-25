#include "entities/Earth.hpp"

#include <utility>

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
	shape.setFillColor(sf::Color(25, 90, 170));
	shape.setOutlineColor(sf::Color(45, 150, 70));
	shape.setOutlineThickness(4.f);
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
