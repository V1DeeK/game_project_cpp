#include "entities/Asteroid.hpp"

#include <cmath>
#include <random>

#include "collision/Geometry.hpp"
#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "entities/Satellite.hpp"

namespace
{
sf::Vector2f CreateOffScreenSpawnPosition(
	const game::GameContext& context,
	float spawnMargin,
	std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(game::screenOrigin, context.windowWidth);
	std::uniform_real_distribution<float> positionY(game::screenOrigin, context.windowHeight);
	std::uniform_int_distribution<int> edgeDist(game::screenEdgeTop, game::screenEdgeLeft);

	switch (edgeDist(rng))
	{
	case game::screenEdgeTop:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case game::screenEdgeRight:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case game::screenEdgeBottom:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}
} // namespace

namespace game
{
std::vector<Asteroid> Asteroid::CreateInitialFleet(const GameContext& context, std::mt19937& rng)
{
	std::vector<Asteroid> asteroids;
	asteroids.reserve(static_cast<std::size_t>(targetAsteroidCount));
	for (int i = hitPointsDepleted; i < targetAsteroidCount; ++i)
	{
		asteroids.push_back(CreateRandom(context, rng));
	}
	return asteroids;
}

void Asteroid::UpdateAll(
	std::vector<Asteroid>& asteroids,
	float deltaTime,
	const GameContext& context,
	std::mt19937& rng)
{
	for (auto& asteroid : asteroids)
	{
		asteroid.m_shape.move(asteroid.m_velocity * deltaTime);
		asteroid.ClampToScreenWithBounce(context);
	}

	while (static_cast<int>(asteroids.size()) < targetAsteroidCount)
	{
		asteroids.push_back(CreateRandom(context, rng));
	}
}

void Asteroid::ProcessMerges(std::vector<Asteroid>& asteroids)
{
	for (std::size_t firstIndex = hitPointsDepleted; firstIndex < asteroids.size(); ++firstIndex)
	{
		for (std::size_t secondIndex = firstIndex + indexIncrement; secondIndex < asteroids.size();)
		{
			if (!asteroids[firstIndex].IntersectsCircle(
					asteroids[secondIndex].GetCenter(),
					asteroids[secondIndex].GetCollisionRadius()))
			{
				++secondIndex;
				continue;
			}

			asteroids[firstIndex] = Merge(asteroids[firstIndex], asteroids[secondIndex]);
			asteroids.erase(asteroids.begin() + static_cast<std::ptrdiff_t>(secondIndex));
		}
	}
}

Asteroid Asteroid::CreateRandom(const GameContext& context, std::mt19937& rng)
{
	const float radius = Satellite::GetRadius(context) * asteroidRadiusMultiplier;
	const float spawnMargin = radius + asteroidSpawnMarginExtra * context.scale;

	std::uniform_real_distribution<float> speed(asteroidMinSpeed, asteroidMaxSpeed);
	std::uniform_real_distribution<float> direction(screenOrigin, twoPi);

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float asteroidSpeed = speed(rng) * context.scale;
	const float angle = direction(rng);
	const sf::Vector2f velocity(std::cos(angle) * asteroidSpeed, std::sin(angle) * asteroidSpeed);

	sf::CircleShape shape(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
	shape.setPosition(position);
	shape.setFillColor(kColorAsteroidFill);
	shape.setOutlineColor(kColorAsteroidOutline);
	shape.setOutlineThickness(defaultOutlineThickness);

	Asteroid asteroid;
	asteroid.m_shape = std::move(shape);
	asteroid.m_velocity = velocity;
	asteroid.m_hp = baseAsteroidHitPoints;
	asteroid.m_maxHp = baseAsteroidHitPoints;
	asteroid.m_mergeCount = initialMergeCount;
	return asteroid;
}

Asteroid Asteroid::Merge(const Asteroid& first, const Asteroid& second)
{
	const float radiusA = first.m_shape.getRadius();
	const float radiusB = second.m_shape.getRadius();
	const float massA = radiusA;
	const float massB = radiusB;
	const float totalMassBeforeLoss = massA + massB;
	const float mergedMass = totalMassBeforeLoss * asteroidMassRetention;

	const sf::Vector2f positionA = first.m_shape.getPosition();
	const sf::Vector2f positionB = second.m_shape.getPosition();
	const sf::Vector2f mergedPosition = (massA * positionA + massB * positionB) / totalMassBeforeLoss;

	sf::Vector2f mergedVelocity = (massA * first.m_velocity + massB * second.m_velocity) / totalMassBeforeLoss;
	mergedVelocity *= asteroidSpeedAfterMerge;

	const float mergedRadius = mergedMass;
	const int mergedCount = first.m_mergeCount + second.m_mergeCount;

	sf::CircleShape shape(mergedRadius);
	shape.setOrigin(sf::Vector2f(mergedRadius, mergedRadius));
	shape.setPosition(mergedPosition);
	shape.setFillColor(kColorAsteroidFill);
	shape.setOutlineColor(kColorAsteroidOutline);
	shape.setOutlineThickness(defaultOutlineThickness);

	Asteroid asteroid;
	asteroid.m_shape = std::move(shape);
	asteroid.m_velocity = mergedVelocity;
	asteroid.m_hp = baseAsteroidHitPoints * mergedCount;
	asteroid.m_maxHp = baseAsteroidHitPoints * mergedCount;
	asteroid.m_mergeCount = mergedCount;
	return asteroid;
}

void Asteroid::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

sf::Vector2f Asteroid::GetCenter() const
{
	return m_shape.getPosition();
}

float Asteroid::GetCollisionRadius() const
{
	return m_shape.getRadius();
}

bool Asteroid::IsOnScreen(const GameContext& context) const
{
	return IsShapeOnScreen(m_shape, context);
}

bool Asteroid::TakeDamage(int damage)
{
	m_hp -= damage;
	if (m_hp <= hitPointsDepleted)
	{
		return true;
	}

	UpdateDamageVisual();
	return false;
}

void Asteroid::UpdateDamageVisual()
{
	if (m_hp < m_maxHp)
	{
		m_shape.setFillColor(kColorAsteroidDamaged);
	}
	else
	{
		m_shape.setFillColor(kColorAsteroidFill);
	}
}

bool Asteroid::IntersectsCircle(sf::Vector2f center, float radius) const
{
	return CirclesIntersect(GetCenter(), GetCollisionRadius(), center, radius);
}

bool Asteroid::IntersectsRect(const sf::FloatRect& rect) const
{
	return CircleIntersectsRect(GetCenter(), GetCollisionRadius(), rect);
}

bool Asteroid::ContainsPoint(sf::Vector2f point) const
{
	return CircleContainsPoint(GetCenter(), GetCollisionRadius(), point);
}

void Asteroid::ClampToScreenWithBounce(const GameContext& context)
{
	sf::FloatRect bounds = m_shape.getGlobalBounds();

	if (bounds.position.x < screenOrigin)
	{
		m_shape.move(sf::Vector2f(-bounds.position.x, screenOrigin));
		m_velocity.x = std::abs(m_velocity.x);
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		m_shape.move(sf::Vector2f(offset, screenOrigin));
		m_velocity.x = -std::abs(m_velocity.x);
	}

	bounds = m_shape.getGlobalBounds();

	if (bounds.position.y < screenOrigin)
	{
		m_shape.move(sf::Vector2f(screenOrigin, -bounds.position.y));
		m_velocity.y = std::abs(m_velocity.y);
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		m_shape.move(sf::Vector2f(screenOrigin, offset));
		m_velocity.y = -std::abs(m_velocity.y);
	}
}
} // namespace game
