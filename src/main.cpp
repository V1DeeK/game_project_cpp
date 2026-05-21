#include <algorithm>
#include <optional>

#include <SFML/Graphics.hpp>

namespace
{
constexpr float windowWidth = 800.f;
constexpr float windowHeight = 600.f;
constexpr float rectangleSize = 20.f;
constexpr float moveSpeed = 250.f;

bool IsMoveKeyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Keyboard::isKeyPressed(scancode);
}

void ClampRectanglePosition(sf::RectangleShape& rectangle)
{
	sf::Vector2f position = rectangle.getPosition();
	position.x = std::clamp(position.x, 0.f, windowWidth - rectangleSize);
	position.y = std::clamp(position.y, 0.f, windowHeight - rectangleSize);
	rectangle.setPosition(position);
}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800u, 600u }), "GAME");
	window.setFramerateLimit(60);
	(void)window.setActive(true);
	window.requestFocus();

	sf::RectangleShape rectangle(sf::Vector2f(rectangleSize, rectangleSize));
	rectangle.setFillColor(sf::Color::White);
	rectangle.setPosition(sf::Vector2f(390.f, 290.f));

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
			offset.x -= moveSpeed * deltaTime;
		}
		if (moveRight || IsMoveKeyPressed(sf::Keyboard::Scancode::D))
		{
			offset.x += moveSpeed * deltaTime;
		}
		if (moveUp || IsMoveKeyPressed(sf::Keyboard::Scancode::W))
		{
			offset.y -= moveSpeed * deltaTime;
		}
		if (moveDown || IsMoveKeyPressed(sf::Keyboard::Scancode::S))
		{
			offset.y += moveSpeed * deltaTime;
		}

		rectangle.move(offset);
		ClampRectanglePosition(rectangle);

		window.clear(sf::Color(30, 30, 40));
		window.draw(rectangle);
		window.display();
	}

	return 0;
}
