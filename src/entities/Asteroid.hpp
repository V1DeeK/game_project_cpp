#pragma once

#include <random>
#include <vector>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"

namespace game
{
class Asteroid
{
public:
	static std::vector<Asteroid> CreateInitialFleet(const GameContext& context, std::mt19937& rng);
	static void UpdateAll(
		std::vector<Asteroid>& asteroids,
		float deltaTime,
		const GameContext& context,
		std::mt19937& rng);
	static void ProcessMerges(std::vector<Asteroid>& asteroids);

	void Draw(sf::RenderWindow& window) const;

	sf::Vector2f GetCenter() const;
	float GetCollisionRadius() const;
	bool IsOnScreen(const GameContext& context) const;
	bool TakeDamage(int damage);
	void UpdateDamageVisual();
	bool IntersectsCircle(sf::Vector2f center, float radius) const;
	bool IntersectsRect(const sf::FloatRect& rect) const;
	bool ContainsPoint(sf::Vector2f point) const;

private:
	static Asteroid CreateRandom(const GameContext& context, std::mt19937& rng);
	static Asteroid Merge(const Asteroid& first, const Asteroid& second);

	void ClampToScreenWithBounce(const GameContext& context);

	sf::CircleShape m_shape;
	sf::Vector2f m_velocity;
	int m_hp = 0;
	int m_maxHp = 0;
	int m_mergeCount = 1;
};
} // namespace game
