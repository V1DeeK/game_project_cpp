#pragma once

#include <vector>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "app/GameContext.hpp"

namespace game
{
class FiringSystem;

class Bullet
{
	friend class FiringSystem;

public:
	static void UpdateAll(std::vector<Bullet>& bullets, float deltaTime, const GameContext& context);

	static void RemoveOffScreen(std::vector<Bullet>& bullets, const GameContext& context);

	void Draw(sf::RenderWindow& window) const;

	sf::Vector2f GetCenter() const;
	sf::FloatRect GetBounds() const;

	bool IntersectsConvexShape(const sf::ConvexShape& shape) const;

private:
	void Update(float deltaTime);

	bool IsOffScreen(const GameContext& context) const;

	sf::RectangleShape m_shape;
	sf::Vector2f m_velocity;
};
} // namespace game
