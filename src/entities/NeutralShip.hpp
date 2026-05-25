#pragma once

#include <random>
#include <vector>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"

namespace game
{
class NeutralShip
{
public:
	static std::vector<NeutralShip> CreateInitialFleet(const GameContext& context, std::mt19937& rng);
	static void UpdateAll(
		std::vector<NeutralShip>& neutrals,
		float deltaTime,
		const GameContext& context,
		std::mt19937& rng);

	void Draw(sf::RenderWindow& window) const;

	bool IsOnScreen(const GameContext& context) const;
	sf::FloatRect GetBounds() const;
	const sf::ConvexShape& GetShape() const;
	bool IntersectsRect(const sf::FloatRect& rect) const;

private:
	static NeutralShip CreateRandom(const GameContext& context, std::mt19937& rng);

	sf::ConvexShape m_shape;
	sf::Vector2f m_velocity;
};
} // namespace game
