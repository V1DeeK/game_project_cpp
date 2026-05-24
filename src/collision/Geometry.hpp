#pragma once

#include <algorithm>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"

namespace game
{
inline bool CircleContainsPoint(sf::Vector2f center, float radius, sf::Vector2f point)
{
	const float dx = point.x - center.x;
	const float dy = point.y - center.y;
	return dx * dx + dy * dy <= radius * radius;
}

inline bool CirclesIntersect(sf::Vector2f centerA, float radiusA, sf::Vector2f centerB, float radiusB)
{
	const float dx = centerA.x - centerB.x;
	const float dy = centerA.y - centerB.y;
	const float combinedRadius = radiusA + radiusB;
	return dx * dx + dy * dy <= combinedRadius * combinedRadius;
}

inline bool CircleIntersectsRect(sf::Vector2f circleCenter, float radius, const sf::FloatRect& rect)
{
	const float closestX = std::clamp(circleCenter.x, rect.position.x, rect.position.x + rect.size.x);
	const float closestY = std::clamp(circleCenter.y, rect.position.y, rect.position.y + rect.size.y);
	const float dx = circleCenter.x - closestX;
	const float dy = circleCenter.y - closestY;
	return dx * dx + dy * dy <= radius * radius;
}

inline bool RectsIntersect(const sf::FloatRect& first, const sf::FloatRect& second)
{
	return first.position.x < second.position.x + second.size.x
		&& first.position.x + first.size.x > second.position.x
		&& first.position.y < second.position.y + second.size.y
		&& first.position.y + first.size.y > second.position.y;
}

inline bool IsIntersectingScreen(const sf::FloatRect& bounds, const GameContext& context)
{
	return bounds.position.x < context.windowWidth
		&& bounds.position.x + bounds.size.x > 0.f
		&& bounds.position.y < context.windowHeight
		&& bounds.position.y + bounds.size.y > 0.f;
}

inline bool IsShapeOnScreen(const sf::Shape& shape, const GameContext& context)
{
	return IsIntersectingScreen(shape.getGlobalBounds(), context);
}

inline bool IsFullyOutsideScreen(const sf::FloatRect& bounds, const GameContext& context, float margin)
{
	return bounds.position.x + bounds.size.x < -margin
		|| bounds.position.x > context.windowWidth + margin
		|| bounds.position.y + bounds.size.y < -margin
		|| bounds.position.y > context.windowHeight + margin;
}
}

