#include "app/GameContext.hpp"

#include <algorithm>

#include "config/GameConstants.hpp"

namespace game
{
GameContext CreateGameContext(const sf::VideoMode& desktopMode)
{
	GameContext context;
	context.windowWidth = static_cast<float>(desktopMode.size.x);
	context.windowHeight = static_cast<float>(desktopMode.size.y);
	context.scale = std::min(context.windowWidth / 800.f, context.windowHeight / 600.f);

	context.shipWidth = 16.f * context.scale * shipVisualScale;
	context.shipHeight = 20.f * context.scale * shipVisualScale;
	context.moveSpeed = 250.f * context.scale;
	context.earthRadius = 80.f * context.scale;
	context.bulletSpeed = 500.f * context.scale;

	return context;
}

sf::Vector2f GetEarthCenter(const GameContext& context)
{
	return sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f);
}
}
