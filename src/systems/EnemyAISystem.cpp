#include "systems/EnemyAISystem.hpp"

#include <cmath>
#include <optional>

#include "config/GameConstants.hpp"
#include "entities/Satellite.hpp"
#include "systems/FiringSystem.hpp"

namespace game
{
void EnemyAISystem::UpdateAll(
	std::vector<EnemyShip>& enemies,
	std::vector<Bullet>& bullets,
	const std::vector<Satellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	sf::Vector2f playerPosition,
	const GameContext& context)
{
	const float moveSpeed = enemyMoveSpeed * context.scale;
	const float arrivalDistance = 4.f * context.scale;

	for (auto& enemy : enemies)
	{
		const sf::Vector2f currentPosition = enemy.GetPosition();

		if (enemy.GetBehavior() == EnemyBehavior::HuntPlayer)
		{
			sf::Vector2f toPlayer = playerPosition - currentPosition;
			const float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
			if (distance > arrivalDistance)
			{
				toPlayer /= distance;
				enemy.Move(toPlayer * moveSpeed * deltaTime);
				enemy.ClampToScreen(context);
			}

			enemy.RotateToward(playerPosition);

			enemy.TickFireCooldown(deltaTime);
			if (enemy.GetFireCooldown() <= 0.f)
			{
				bullets.push_back(FiringSystem::CreateBulletFromShip(enemy, context));
				enemy.SetFireCooldown(enemyFireInterval);
			}
			continue;
		}

		sf::Vector2f toTarget = enemy.GetTargetPosition() - currentPosition;
		const float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

		if (distance > arrivalDistance)
		{
			toTarget /= distance;
			enemy.Move(toTarget * moveSpeed * deltaTime);
			enemy.ClampToScreen(context);
			enemy.RotateTowardFromPosition(currentPosition, earthCenter);
			continue;
		}

		const std::optional<sf::Vector2f> satelliteTarget = Satellite::FindNearestPosition(currentPosition, satellites);
		if (!satelliteTarget.has_value())
		{
			enemy.RotateTowardFromPosition(currentPosition, earthCenter);
			continue;
		}

		enemy.RotateTowardFromPosition(currentPosition, *satelliteTarget);

		enemy.TickFireCooldown(deltaTime);
		if (enemy.GetFireCooldown() <= 0.f)
		{
			bullets.push_back(FiringSystem::CreateBulletFromShip(enemy, context));
			enemy.SetFireCooldown(enemyFireInterval);
		}
	}
}
} // namespace game
