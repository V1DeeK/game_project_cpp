#include <algorithm>
#include <cmath>
#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

struct GameContext
{
	float windowWidth = 800.f;
	float windowHeight = 600.f;
	float shipWidth = 16.f;
	float shipHeight = 20.f;
	float moveSpeed = 250.f;
	float earthRadius = 120.f;
	float scale = 1.f;
};

struct FlyingObject
{
	sf::CircleShape shape;
	sf::Vector2f velocity;
};

namespace
{
constexpr float shipVisualScale = 0.45f;
constexpr float spawnIntervalSeconds = 0.35f;

bool IsMoveKeyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Keyboard::isKeyPressed(scancode);
}

GameContext CreateGameContext(const sf::VideoMode& desktopMode)
{
	GameContext context;
	context.windowWidth = static_cast<float>(desktopMode.size.x);
	context.windowHeight = static_cast<float>(desktopMode.size.y);
	context.scale = std::min(context.windowWidth / 800.f, context.windowHeight / 600.f);

	context.shipWidth = 16.f * context.scale * shipVisualScale;
	context.shipHeight = 20.f * context.scale * shipVisualScale;
	context.moveSpeed = 250.f * context.scale;
	context.earthRadius = 120.f * context.scale;

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

sf::Color RandomDebrisColor(std::mt19937& rng)
{
	std::uniform_int_distribution<int> channel(120, 220);
	return sf::Color(
		static_cast<std::uint8_t>(channel(rng)),
		static_cast<std::uint8_t>(channel(rng)),
		static_cast<std::uint8_t>(channel(rng)));
}

FlyingObject CreateRandomFlyingObject(const GameContext& context, std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(0.f, context.windowWidth);
	std::uniform_real_distribution<float> positionY(0.f, context.windowHeight);
	std::uniform_real_distribution<float> speed(120.f, 320.f);
	std::uniform_real_distribution<float> direction(0.f, 6.2831853f);
	std::uniform_real_distribution<float> radiusDist(6.f, 18.f);
	std::uniform_int_distribution<int> edgeDist(0, 3);

	const float margin = 30.f * context.scale;
	const float radius = radiusDist(rng) * context.scale;

	sf::Vector2f position;
	switch (edgeDist(rng))
	{
	case 0:
		position = sf::Vector2f(positionX(rng), -margin);
		break;
	case 1:
		position = sf::Vector2f(context.windowWidth + margin, positionY(rng));
		break;
	case 2:
		position = sf::Vector2f(positionX(rng), context.windowHeight + margin);
		break;
	default:
		position = sf::Vector2f(-margin, positionY(rng));
		break;
	}

	const float objectSpeed = speed(rng) * context.scale;
	const float angle = direction(rng);
	const sf::Vector2f velocity(std::cos(angle) * objectSpeed, std::sin(angle) * objectSpeed);

	sf::CircleShape shape(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
	shape.setPosition(position);
	shape.setFillColor(RandomDebrisColor(rng));
	shape.setOutlineColor(sf::Color(60, 60, 70));
	shape.setOutlineThickness(1.f);

	return FlyingObject{ shape, velocity };
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

void UpdateFlyingObjects(std::vector<FlyingObject>& objects, float deltaTime, const GameContext& context)
{
	const float removeMargin = 80.f * context.scale;

	for (auto& object : objects)
	{
		object.shape.move(object.velocity * deltaTime);
	}

	objects.erase(
		std::remove_if(
			objects.begin(),
			objects.end(),
			[&](const FlyingObject& object) {
				const sf::Vector2f position = object.shape.getPosition();
				return position.x < -removeMargin
					|| position.x > context.windowWidth + removeMargin
					|| position.y < -removeMargin
					|| position.y > context.windowHeight + removeMargin;
			}),
		objects.end());
}
} // namespace

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

	std::mt19937 rng(std::random_device{}());
	std::vector<FlyingObject> flyingObjects;
	flyingObjects.reserve(128);

	float spawnTimer = 0.f;

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

		spawnTimer += deltaTime;
		if (spawnTimer >= spawnIntervalSeconds)
		{
			spawnTimer = 0.f;
			flyingObjects.push_back(CreateRandomFlyingObject(context, rng));
		}

		UpdateFlyingObjects(flyingObjects, deltaTime, context);

		window.clear(sf::Color(10, 10, 25));
		window.draw(earth);
		for (const auto& object : flyingObjects)
		{
			window.draw(object.shape);
		}
		window.draw(spaceship);
		window.display();
	}

	return 0;
}
