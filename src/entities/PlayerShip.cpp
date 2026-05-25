#include "entities/PlayerShip.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <utility>

#include "config/Colors.hpp"

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
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setOrigin(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight / 2.f));
	ship.setFillColor(game::kColorPlayerShipFill);
	ship.setOutlineColor(game::kColorPlayerShipOutline);
	ship.setOutlineThickness(1.f);
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
	player.SetPosition(sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f));
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

	if (bounds.position.x < 0.f)
	{
		shape.move(sf::Vector2f(-bounds.position.x, 0.f));
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, 0.f));
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < 0.f)
	{
		shape.move(sf::Vector2f(0.f, -bounds.position.y));
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(0.f, offset));
	}
}
} // namespace game
