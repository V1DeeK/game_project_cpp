#include "entities/Satellite.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "collision/Geometry.hpp"
#include "config/Colors.hpp"

namespace game
{
float Satellite::GetRadius(const GameContext& context)
{
	return 5.f * context.scale;
}

std::optional<sf::Vector2f> Satellite::FindNearestPosition(
	sf::Vector2f fromPosition,
	const std::vector<Satellite>& satellites)
{
	if (satellites.empty())
	{
		return std::nullopt;
	}

	std::optional<sf::Vector2f> nearestPosition;
	float nearestDistanceSquared = std::numeric_limits<float>::max();

	for (const auto& satellite : satellites)
	{
		const sf::Vector2f satellitePosition = satellite.GetCenter();
		const sf::Vector2f delta = satellitePosition - fromPosition;
		const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestPosition = satellitePosition;
		}
	}

	return nearestPosition;
}

void Satellite::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

void Satellite::DrawHpBar(sf::RenderWindow& window, const GameContext& context) const
{
	const float radius = m_shape.getRadius();
	const sf::Vector2f center = m_shape.getPosition();
	const float barWidth = 18.f * context.scale;
	const float barHeight = 3.f * context.scale;
	const sf::Vector2f barPosition(center.x - barWidth / 2.f, center.y - radius - 7.f * context.scale);

	sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
	background.setPosition(barPosition);
	background.setFillColor(kColorHpBackground);

	const float hpFraction = static_cast<float>(m_hp) / static_cast<float>(m_maxHp);
	const float foregroundWidth = barWidth * hpFraction;

	sf::RectangleShape foreground(sf::Vector2f(foregroundWidth, barHeight));
	foreground.setPosition(barPosition);
	if (m_hp == m_maxHp)
	{
		foreground.setFillColor(kColorHpForeground);
	}
	else if (m_hp > 0)
	{
		foreground.setFillColor(kColorHpDamaged);
	}

	window.draw(background);
	if (foregroundWidth > 0.f)
	{
		window.draw(foreground);
	}
}

sf::Vector2f Satellite::GetCenter() const
{
	return m_shape.getPosition();
}

float Satellite::GetCollisionRadius() const
{
	return m_shape.getRadius();
}

bool Satellite::TakeDamage(int damage)
{
	m_hp -= damage;
	return m_hp <= 0;
}

bool Satellite::IntersectsCircle(sf::Vector2f center, float radius) const
{
	return CirclesIntersect(GetCenter(), GetCollisionRadius(), center, radius);
}

bool Satellite::IntersectsRect(const sf::FloatRect& rect) const
{
	return CircleIntersectsRect(GetCenter(), GetCollisionRadius(), rect);
}

bool Satellite::ContainsPoint(sf::Vector2f point) const
{
	return CircleContainsPoint(GetCenter(), GetCollisionRadius(), point);
}

void Satellite::SetOrbitPosition(sf::Vector2f earthCenter)
{
	const float x = earthCenter.x + std::cos(m_angle) * m_orbitRadius;
	const float y = earthCenter.y + std::sin(m_angle) * m_orbitRadius;
	m_shape.setPosition(sf::Vector2f(x, y));
}

void Satellite::ClampToScreen(const GameContext& context)
{
	const float radius = m_shape.getRadius();
	sf::Vector2f position = m_shape.getPosition();
	position.x = std::clamp(position.x, radius, context.windowWidth - radius);
	position.y = std::clamp(position.y, radius, context.windowHeight - radius);
	m_shape.setPosition(position);
}
} // namespace game
