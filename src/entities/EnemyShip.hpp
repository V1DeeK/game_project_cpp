#pragma once

#include <random>
#include <vector>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "entities/Ship.hpp"

namespace game
{
enum class EnemyBehavior
{
	AttackSatellites,
	HuntPlayer,
};

class EnemyShip : public Ship
{
public:
	explicit EnemyShip(Ship ship);

	static void SpawnFromAllSides(
		std::vector<EnemyShip>& enemies,
		const GameContext& context,
		sf::Vector2f earthCenter,
		std::mt19937& rng);

	bool IsOnScreen(const GameContext& context) const;
	bool IntersectsRect(const sf::FloatRect& rect) const;
	void ClampToScreen(const GameContext& context);
	void RotateTowardFromPosition(sf::Vector2f fromPosition, sf::Vector2f targetPoint);

	EnemyBehavior GetBehavior() const;
	sf::Vector2f GetTargetPosition() const;
	int GetTargetRingIndex() const;
	int GetTargetPointIndex() const;
	float GetFireCooldown() const;
	void SetFireCooldown(float cooldown);
	void TickFireCooldown(float deltaTime);

private:
	static sf::Vector2f CreateSpawnPositionFromEdge(int edge, const GameContext& context, float spawnMargin, std::mt19937& rng);

	sf::Vector2f m_targetPosition;
	int m_targetRingIndex = 0;
	int m_targetPointIndex = 0;
	float m_fireCooldown = 0.f;
	EnemyBehavior m_behavior = EnemyBehavior::AttackSatellites;
};
} // namespace game
