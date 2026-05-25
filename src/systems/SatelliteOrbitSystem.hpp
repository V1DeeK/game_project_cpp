#pragma once

#include <vector>

#include <SFML/System/Vector2.hpp>

#include "app/GameContext.hpp"
#include "entities/Satellite.hpp"

namespace game
{
class SatelliteOrbitSystem
{
public:
	static std::vector<Satellite> CreateOrbitSatellites(const GameContext& context);
	static void UpdateOrbitSatellites(
		std::vector<Satellite>& satellites,
		float deltaTime,
		sf::Vector2f earthCenter,
		const GameContext& context);
};
} // namespace game
