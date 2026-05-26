#pragma once

#include <optional>
#include <vector>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "config/GameConstants.hpp"

namespace game
{
class SatelliteOrbitSystem;

class Satellite
{
	friend class SatelliteOrbitSystem;

public:
	static float GetRadius(const GameContext& context);
	static std::optional<sf::Vector2f> FindNearestPosition(
		sf::Vector2f fromPosition,
		const std::vector<Satellite>& satellites);

	void Draw(sf::RenderWindow& window) const;
	void DrawHpBar(sf::RenderWindow& window, const GameContext& context) const;

	sf::Vector2f GetCenter() const;
	float GetCollisionRadius() const;
	bool TakeDamage(int damage);
	bool IntersectsCircle(sf::Vector2f center, float radius) const;
	bool IntersectsRect(const sf::FloatRect& rect) const;
	bool ContainsPoint(sf::Vector2f point) const;

private:
	void SetOrbitPosition(sf::Vector2f earthCenter);
	void ClampToScreen(const GameContext& context);

	sf::CircleShape m_shape;
	float m_angle = initialOrbitAngle;
	float m_angularSpeed = initialAngularSpeed;
	float m_orbitRadius = initialOrbitRadius;
	int m_hp = initialHitPoints;
	int m_maxHp = initialHitPoints;
};
} // namespace game
