#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "config/GameConstants.hpp"

namespace game
{
struct GameContext
{
	float windowWidth = referenceWindowWidth;
	float windowHeight = referenceWindowHeight;
	float shipWidth = baseShipWidth;
	float shipHeight = baseShipHeight;
	float moveSpeed = baseMoveSpeed;
	float earthRadius = baseEarthRadius;
	float scale = unitScale;
	float bulletSpeed = baseBulletSpeed;
};

GameContext CreateGameContext(const sf::VideoMode& desktopMode);
sf::Vector2f GetEarthCenter(const GameContext& context);
} // namespace game
