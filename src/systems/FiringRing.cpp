#include "systems/FiringRing.hpp"

#include <cmath>
#include <limits>

#include "config/GameConstants.hpp"
#include "entities/EnemyShip.hpp"

namespace
{
float GetSatelliteOrbitRadius(const game::GameContext& context)
{
	return context.earthRadius + 24.f * context.scale;
}

float GetFiringRingRadius(const game::GameContext& context, int ringIndex)
{
	return GetSatelliteOrbitRadius(context)
		+ (game::firingRingOffset + game::firingRingSpacing * static_cast<float>(ringIndex)) * context.scale;
}

bool IsFiringSlotOccupied(int ringIndex, int pointIndex, const std::vector<game::EnemyShip>& enemies)
{
	for (const auto& enemy : enemies)
	{
		if (enemy.GetTargetRingIndex() == ringIndex && enemy.GetTargetPointIndex() == pointIndex)
		{
			return true;
		}
	}
	return false;
}

bool HasFreeFiringSlotOnRing(int ringIndex, const std::vector<game::EnemyShip>& enemies)
{
	for (int pointIndex = 0; pointIndex < game::firingPointCount; ++pointIndex)
	{
		if (!IsFiringSlotOccupied(ringIndex, pointIndex, enemies))
		{
			return true;
		}
	}
	return false;
}
} // namespace

namespace game
{
sf::Vector2f FiringRing::GetPointPosition(
	sf::Vector2f earthCenter,
	const GameContext& context,
	int ringIndex,
	int pointIndex)
{
	const float firingRadius = GetFiringRingRadius(context, ringIndex);
	const float angleStep = 6.2831853f / static_cast<float>(firingPointCount);
	const float ringAngleOffset = firingRing2AngleOffsetDegrees * 3.14159265f / 180.f
		* static_cast<float>(ringIndex);
	const float angle = angleStep * static_cast<float>(pointIndex) + ringAngleOffset;
	return sf::Vector2f(
		earthCenter.x + std::cos(angle) * firingRadius,
		earthCenter.y + std::sin(angle) * firingRadius);
}

FiringSlotAssignment FiringRing::FindNearestSlot(
	sf::Vector2f spawnPosition,
	sf::Vector2f earthCenter,
	const GameContext& context,
	const std::vector<EnemyShip>& enemies)
{
	FiringSlotAssignment result;
	int searchRingCount = firingRingCount;

	for (int ringIndex = 0; ringIndex < firingRingCount; ++ringIndex)
	{
		if (HasFreeFiringSlotOnRing(ringIndex, enemies))
		{
			searchRingCount = ringIndex + 1;
			break;
		}
	}

	int bestFreeRing = -1;
	int bestFreePoint = 0;
	float bestFreeDistance = std::numeric_limits<float>::max();
	int bestAnyRing = 0;
	int bestAnyPoint = 0;
	float bestAnyDistance = std::numeric_limits<float>::max();

	for (int ringIndex = 0; ringIndex < searchRingCount; ++ringIndex)
	{
		for (int pointIndex = 0; pointIndex < firingPointCount; ++pointIndex)
		{
			const sf::Vector2f slotPosition = GetPointPosition(earthCenter, context, ringIndex, pointIndex);
			const sf::Vector2f delta = slotPosition - spawnPosition;
			const float distance = delta.x * delta.x + delta.y * delta.y;

			if (distance < bestAnyDistance)
			{
				bestAnyDistance = distance;
				bestAnyRing = ringIndex;
				bestAnyPoint = pointIndex;
			}

			if (!IsFiringSlotOccupied(ringIndex, pointIndex, enemies) && distance < bestFreeDistance)
			{
				bestFreeDistance = distance;
				bestFreeRing = ringIndex;
				bestFreePoint = pointIndex;
			}
		}
	}

	if (bestFreeRing >= 0)
	{
		result.ringIndex = bestFreeRing;
		result.pointIndex = bestFreePoint;
	}
	else
	{
		result.ringIndex = bestAnyRing;
		result.pointIndex = bestAnyPoint;
	}

	return result;
}
} // namespace game
