#include "app/GameContext.hpp"

#include <algorithm>

namespace game
{
GameContext CreateGameContext(const sf::VideoMode& desktopMode)
{
	GameContext context;
	context.windowWidth = static_cast<float>(desktopMode.size.x);
	context.windowHeight = static_cast<float>(desktopMode.size.y);
	context.scale = std::min(
		context.windowWidth / referenceWindowWidth,
		context.windowHeight / referenceWindowHeight);

	context.shipWidth = baseShipWidth * context.scale * shipVisualScale;
	context.shipHeight = baseShipHeight * context.scale * shipVisualScale;
	context.moveSpeed = baseMoveSpeed * context.scale;
	context.earthRadius = baseEarthRadius * context.scale;
	context.bulletSpeed = baseBulletSpeed * context.scale;

	return context;
}

sf::Vector2f GetEarthCenter(const GameContext& context)
{
	return sf::Vector2f(context.windowWidth * half, context.windowHeight * half);
}
} // namespace game
