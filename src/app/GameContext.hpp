#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>

namespace game
{
struct GameContext
{
	float windowWidth = 800.f;
	float windowHeight = 600.f;
	float shipWidth = 16.f;
	float shipHeight = 20.f;
	float moveSpeed = 250.f;
	float earthRadius = 80.f;
	float scale = 1.f;
	float bulletSpeed = 500.f;
};

GameContext CreateGameContext(const sf::VideoMode& desktopMode);
sf::Vector2f GetEarthCenter(const GameContext& context);
}