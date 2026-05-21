#include <algorithm>
#include <cmath>
#include <optional>

#include <SFML/Graphics.hpp>

struct GameContext
{
	float windowWidth = 800.f;
	float windowHeight = 600.f;
	float shipWidth = 16.f;
	float shipHeight = 20.f;
	float moveSpeed = 250.f;
	float earthRadius = 70.f;
};

namespace
{
bool IsMoveKeyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Keyboard::isKeyPressed(scancode);
}

GameContext CreateGameContext(const sf::VideoMode& desktopMode)
{
	GameContext context;
	context.windowWidth = static_cast<float>(desktopMode.size.x);
	context.windowHeight = static_cast<float>(desktopMode.size.y);

	const float scale = std::min(context.windowWidth / 800.f, context.windowHeight / 600.f);
	context.shipWidth = 16.f * scale;
	context.shipHeight = 20.f * scale;
	context.moveSpeed = 250.f * scale;
	context.earthRadius = 120.f * scale;

	return context;
}

sf::ConvexShape CreateSpaceship(const GameContext& context)
{
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setFillColor(sf::Color(220, 220, 230));
	ship.setOutlineColor(sf::Color(80, 200, 255));
	ship.setOutlineThickness(1.f);
	return ship;
}

sf::CircleShape CreateEarth(const GameContext& context)
{
	sf::CircleShape earth(context.earthRadius);
	earth.setOrigin(sf::Vector2f(context.earthRadius, context.earthRadius));
	earth.setPosition(sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f));
	earth.setFillColor(sf::Color(25, 90, 170));
	earth.setOutlineColor(sf::Color(45, 150, 70));
	earth.setOutlineThickness(4.f);
	return earth;
}

void ClampShipPosition(sf::ConvexShape& ship, const GameContext& context)
{
	sf::Vector2f position = ship.getPosition();
	position.x = std::clamp(position.x, 0.f, context.windowWidth - context.shipWidth);
	position.y = std::clamp(position.y, 0.f, context.windowHeight - context.shipHeight);
	ship.setPosition(position);
}

void UpdateShipRotation(sf::ConvexShape& ship, sf::Vector2f movement)
{
	if (movement.x == 0.f && movement.y == 0.f)
	{
		return;
	}

	const float angleDegrees = std::atan2(movement.y, movement.x) * 180.f / 3.14159265f + 90.f;
	ship.setRotation(sf::degrees(angleDegrees));
}
}

int main()
{
	const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
	const GameContext context = CreateGameContext(desktopMode);

	sf::RenderWindow window(desktopMode, "GAME", sf::State::Fullscreen);
	window.setFramerateLimit(60);
	(void)window.setActive(true);
	window.requestFocus();

	sf::CircleShape earth = CreateEarth(context);
	sf::ConvexShape spaceship = CreateSpaceship(context);
	spaceship.setPosition(sf::Vector2f(
		context.windowWidth / 2.f - context.shipWidth / 2.f,
		context.windowHeight / 2.f - context.shipHeight / 2.f));

	sf::Clock clock;

	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;

	while (window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		deltaTime = std::clamp(deltaTime, 1.f / 500.f, 0.05f);

		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
			else if (event->is<sf::Event::MouseButtonPressed>())
			{
				window.requestFocus();
			}
			else if (event->is<sf::Event::FocusGained>())
			{
				window.requestFocus();
			}
			else if (event->is<sf::Event::FocusLost>())
			{
				moveLeft = false;
				moveRight = false;
				moveUp = false;
				moveDown = false;
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				if (keyPressed->code == sf::Keyboard::Key::Escape)
				{
					window.close();
				}

				switch (keyPressed->scancode)
				{
				case sf::Keyboard::Scancode::A:
					moveLeft = true;
					break;
				case sf::Keyboard::Scancode::D:
					moveRight = true;
					break;
				case sf::Keyboard::Scancode::W:
					moveUp = true;
					break;
				case sf::Keyboard::Scancode::S:
					moveDown = true;
					break;
				default:
					break;
				}
			}
			else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
			{
				switch (keyReleased->scancode)
				{
				case sf::Keyboard::Scancode::A:
					moveLeft = false;
					break;
				case sf::Keyboard::Scancode::D:
					moveRight = false;
					break;
				case sf::Keyboard::Scancode::W:
					moveUp = false;
					break;
				case sf::Keyboard::Scancode::S:
					moveDown = false;
					break;
				default:
					break;
				}
			}
		}

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

		spaceship.move(offset);
		ClampShipPosition(spaceship, context);
		UpdateShipRotation(spaceship, offset);

		window.clear(sf::Color(10, 10, 25));
		window.draw(earth);
		window.draw(spaceship);
		window.display();
	}

	return 0;
}
