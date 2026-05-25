#include "systems/SatelliteOrbitSystem.hpp"

#include <cmath>

#include "app/GameContext.hpp"
#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "entities/Satellite.hpp"

namespace game
{
std::vector<Satellite> SatelliteOrbitSystem::CreateOrbitSatellites(const GameContext& context)
{
	const float satelliteRadius = Satellite::GetRadius(context);
	const float orbitRadius = context.earthRadius + 24.f * context.scale;
	const sf::Vector2f earthCenter = GetEarthCenter(context);
	const float angleStep = 6.2831853f / static_cast<float>(orbitSatelliteCount);

	std::vector<Satellite> satellites;
	satellites.reserve(orbitSatelliteCount);

	for (int i = 0; i < orbitSatelliteCount; ++i)
	{
		Satellite satellite;
		satellite.m_angle = angleStep * static_cast<float>(i);
		satellite.m_angularSpeed = orbitAngularSpeed;
		satellite.m_orbitRadius = orbitRadius;
		satellite.m_hp = satelliteHitPoints;
		satellite.m_maxHp = satelliteHitPoints;
		satellite.m_shape = sf::CircleShape(satelliteRadius);
		satellite.m_shape.setOrigin(sf::Vector2f(satelliteRadius, satelliteRadius));
		satellite.m_shape.setFillColor(kColorSatelliteFill);
		satellite.m_shape.setOutlineColor(kColorSatelliteOutline);
		satellite.m_shape.setOutlineThickness(1.f);
		satellite.SetOrbitPosition(earthCenter);
		satellites.push_back(satellite);
	}

	return satellites;
}

void SatelliteOrbitSystem::UpdateOrbitSatellites(
	std::vector<Satellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	const GameContext& context)
{
	for (auto& satellite : satellites)
	{
		satellite.m_angle += satellite.m_angularSpeed * deltaTime;
		satellite.SetOrbitPosition(earthCenter);
		satellite.ClampToScreen(context);
	}
}
} // namespace game
