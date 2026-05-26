#include "entities/PlayerShip.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <utility>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"

namespace
{
bool IsMoveKeyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Keyboard::isKeyPressed(scancode);
}

sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window)
{
	const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
	return window.mapPixelToCoords(mousePixel);
}

sf::ConvexShape CreatePlayerShape(const game::GameContext& context)
{
	sf::ConvexShape ship(game::shipTriangleVertexCount);
	ship.setPoint(
		game::triangleVertexNose,
		sf::Vector2f(context.shipWidth * game::half, game::screenOrigin));
	ship.setPoint(game::triangleVertexBottomLeft, sf::Vector2f(game::screenOrigin, context.shipHeight));
	ship.setPoint(game::triangleVertexBottomRight, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth * game::half, context.shipHeight * game::half));
	ship.setFillColor(game::kColorPlayerShipFill);
	ship.setOutlineColor(game::kColorPlayerShipOutline);
	ship.setOutlineThickness(game::defaultOutlineThickness);
	return ship;
}
} // namespace

namespace game
{
PlayerShip::PlayerShip(sf::ConvexShape shape)
	: Ship(std::move(shape))
{
}

PlayerShip PlayerShip::CreateAtCenter(const GameContext& context)
{
	PlayerShip player(CreatePlayerShape(context));
	player.SetPosition(sf::Vector2f(context.windowWidth * half, context.windowHeight * half));
	return player;
}

void PlayerShip::UpdateAim(const sf::RenderWindow& window)
{
	RotateToward(GetMouseWorldPosition(window));
}

void PlayerShip::ApplyMovement(
	bool moveLeft,
	bool moveRight,
	bool moveUp,
	bool moveDown,
	float deltaTime,
	const GameContext& context)
{
	sf::Vector2f offset;
	if (moveLeft || IsMoveKeyPressed(sf::Keyboard::Scancode::A))
	{
		offset.x -= context.moveSpeed * deltaTime;
	}
	if (moveRight || IsMoveKeyPressed(sf::Keyboard::Scancode::D))
	{
		offset.x += context.moveSpeed * deltaTime;
	}
	if (moveUp || IsMoveKeyPressed(sf::Keyboard::Scancode::W))
	{
		offset.y -= context.moveSpeed * deltaTime;
	}
	if (moveDown || IsMoveKeyPressed(sf::Keyboard::Scancode::S))
	{
		offset.y += context.moveSpeed * deltaTime;
	}

	Move(offset);
}

void PlayerShip::ClampToScreen(const GameContext& context)
{
	sf::Shape& shape = GetShape();
	sf::FloatRect bounds = shape.getGlobalBounds();

	if (bounds.position.x < screenOrigin)
	{
		shape.move(sf::Vector2f(-bounds.position.x, screenOrigin));
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, screenOrigin));
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < screenOrigin)
	{
		shape.move(sf::Vector2f(screenOrigin, -bounds.position.y));
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(screenOrigin, offset));
	}
}
} // namespace game
