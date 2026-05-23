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
	float earthRadius = 80.f;
	float scale = 1.f;
	float bulletSpeed = 500.f;
};

struct Asteroid
{
	sf::CircleShape shape;
	sf::Vector2f velocity;
};

struct OrbitSatellite
{
	sf::CircleShape shape;
	float angle = 0.f;
	float angularSpeed = 0.f;
	float orbitRadius = 0.f;
	int hp = 2;
	int maxHp = 2;
};

struct NeutralShip
{
	sf::ConvexShape shape;
	sf::Vector2f velocity;
};

struct Bullet
{
	sf::RectangleShape shape;
	sf::Vector2f velocity;
};

namespace
{
constexpr float shipVisualScale = 0.45f;
constexpr int minAsteroidCount = 10;
constexpr int maxAsteroidCount = 15;
constexpr int orbitSatelliteCount = 8;
constexpr float orbitAngularSpeed = 0.7f;
constexpr int neutralShipCount = 4;
constexpr float neutralShipSizeMultiplier = 5.f;
constexpr float neutralMoveSpeed = 5.f;

constexpr sf::Color kColorPlayerShipFill(50, 120, 230);
constexpr sf::Color kColorPlayerShipOutline(120, 180, 255);
constexpr sf::Color kColorSatelliteFill(55, 95, 220);
constexpr sf::Color kColorSatelliteOutline(100, 150, 255);
constexpr sf::Color kColorAsteroidFill(139, 90, 43);
constexpr sf::Color kColorAsteroidOutline(100, 65, 30);
constexpr sf::Color kColorNeutralFill(35, 130, 55);
constexpr sf::Color kColorNeutralOutline(20, 90, 40);
constexpr sf::Color kColorBullet(255, 220, 50);
constexpr sf::Color kColorHpBackground(90, 20, 20);
constexpr sf::Color kColorHpForeground(0, 200, 0);
constexpr sf::Color kColorHpDamaged(200, 40, 40);

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
	context.earthRadius = 80.f * context.scale;
	context.bulletSpeed = 500.f * context.scale;

	return context;
}

sf::Vector2f GetEarthCenter(const GameContext& context)
{
	return sf::Vector2f(context.windowWidth / 2.f, context.windowHeight / 2.f);
}

float GetSatelliteRadius(const GameContext& context)
{
	return 5.f * context.scale;
}

sf::Vector2f GetNeutralShipSize(const GameContext& context)
{
	return sf::Vector2f(
		context.shipWidth * neutralShipSizeMultiplier,
		context.shipHeight * neutralShipSizeMultiplier);
}

sf::ConvexShape CreatePlayerShip(const GameContext& context)
{
	sf::ConvexShape ship(3);
	ship.setPoint(0, sf::Vector2f(context.shipWidth / 2.f, 0.f));
	ship.setPoint(1, sf::Vector2f(0.f, context.shipHeight));
	ship.setPoint(2, sf::Vector2f(context.shipWidth, context.shipHeight));
	ship.setFillColor(kColorPlayerShipFill);
	ship.setOutlineColor(kColorPlayerShipOutline);
	ship.setOutlineThickness(1.f);
	return ship;
}

sf::CircleShape CreateEarth(const GameContext& context)
{
	sf::CircleShape earth(context.earthRadius);
	earth.setOrigin(sf::Vector2f(context.earthRadius, context.earthRadius));
	earth.setPosition(GetEarthCenter(context));
	earth.setFillColor(sf::Color(25, 90, 170));
	earth.setOutlineColor(sf::Color(45, 150, 70));
	earth.setOutlineThickness(4.f);
	return earth;
}

void SetOrbitSatellitePosition(OrbitSatellite& satellite, sf::Vector2f earthCenter)
{
	const float x = earthCenter.x + std::cos(satellite.angle) * satellite.orbitRadius;
	const float y = earthCenter.y + std::sin(satellite.angle) * satellite.orbitRadius;
	satellite.shape.setPosition(sf::Vector2f(x, y));
}

std::vector<OrbitSatellite> CreateOrbitSatellites(const GameContext& context)
{
	const float satelliteRadius = GetSatelliteRadius(context);
	const float orbitRadius = context.earthRadius + 24.f * context.scale;
	const sf::Vector2f earthCenter = GetEarthCenter(context);
	const float angleStep = 6.2831853f / static_cast<float>(orbitSatelliteCount);

	std::vector<OrbitSatellite> satellites;
	satellites.reserve(orbitSatelliteCount);

	for (int i = 0; i < orbitSatelliteCount; ++i)
	{
		OrbitSatellite satellite;
		satellite.angle = angleStep * static_cast<float>(i);
		satellite.angularSpeed = orbitAngularSpeed;
		satellite.orbitRadius = orbitRadius;
		satellite.hp = 2;
		satellite.maxHp = 2;
		satellite.shape = sf::CircleShape(satelliteRadius);
		satellite.shape.setOrigin(sf::Vector2f(satelliteRadius, satelliteRadius));
		satellite.shape.setFillColor(kColorSatelliteFill);
		satellite.shape.setOutlineColor(kColorSatelliteOutline);
		satellite.shape.setOutlineThickness(1.f);
		SetOrbitSatellitePosition(satellite, earthCenter);
		satellites.push_back(satellite);
	}

	return satellites;
}

void ClampCircleToScreen(sf::CircleShape& circle, const GameContext& context)
{
	const float radius = circle.getRadius();
	sf::Vector2f position = circle.getPosition();
	position.x = std::clamp(position.x, radius, context.windowWidth - radius);
	position.y = std::clamp(position.y, radius, context.windowHeight - radius);
	circle.setPosition(position);
}

void UpdateOrbitSatellites(
	std::vector<OrbitSatellite>& satellites,
	float deltaTime,
	sf::Vector2f earthCenter,
	const GameContext& context)
{
	for (auto& satellite : satellites)
	{
		satellite.angle += satellite.angularSpeed * deltaTime;
		SetOrbitSatellitePosition(satellite, earthCenter);
		ClampCircleToScreen(satellite.shape, context);
	}
}

void DrawSatelliteHpBar(sf::RenderWindow& window, const OrbitSatellite& satellite, const GameContext& context)
{
	const float radius = satellite.shape.getRadius();
	const sf::Vector2f center = satellite.shape.getPosition();
	const float barWidth = 18.f * context.scale;
	const float barHeight = 3.f * context.scale;
	const sf::Vector2f barPosition(center.x - barWidth / 2.f, center.y - radius - 7.f * context.scale);

	sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
	background.setPosition(barPosition);
	background.setFillColor(kColorHpBackground);

	const float hpFraction = static_cast<float>(satellite.hp) / static_cast<float>(satellite.maxHp);
	const float foregroundWidth = barWidth * hpFraction;

	sf::RectangleShape foreground(sf::Vector2f(foregroundWidth, barHeight));
	foreground.setPosition(barPosition);
	if (satellite.hp == satellite.maxHp)
	{
		foreground.setFillColor(kColorHpForeground);
	}
	else if (satellite.hp > 0)
	{
		foreground.setFillColor(kColorHpDamaged);
	}

	window.draw(background);
	if (foregroundWidth > 0.f)
	{
		window.draw(foreground);
	}
}

sf::Vector2f GetShipForwardDirection(const sf::ConvexShape& ship, const GameContext& context)
{
	const sf::Transform transform = ship.getTransform();
	const sf::Vector2f nose = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
	const sf::Vector2f body = transform.transformPoint(sf::Vector2f(context.shipWidth / 2.f, context.shipHeight * 0.5f));
	sf::Vector2f direction = nose - body;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.0001f)
	{
		direction /= length;
	}
	return direction;
}

sf::Vector2f GetShipNosePosition(const sf::ConvexShape& ship, const GameContext& context)
{
	return ship.getTransform().transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
}

Bullet CreateBulletFromShip(const sf::ConvexShape& ship, const GameContext& context)
{
	const sf::Vector2f direction = GetShipForwardDirection(ship, context);
	const sf::Vector2f nosePosition = GetShipNosePosition(ship, context);

	const float bulletWidth = 2.f * context.scale;
	const float bulletLength = 5.f * context.scale;

	sf::RectangleShape shape(sf::Vector2f(bulletWidth, bulletLength));
	shape.setOrigin(sf::Vector2f(bulletWidth / 2.f, bulletLength / 2.f));
	shape.setPosition(nosePosition + direction * (bulletLength * 0.6f));
	shape.setFillColor(kColorBullet);

	const float angleDegrees = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f + 90.f;
	shape.setRotation(sf::degrees(angleDegrees));

	return Bullet{ shape, direction * context.bulletSpeed };
}

sf::Vector2f CreateOffScreenSpawnPosition(
	const GameContext& context,
	float spawnMargin,
	std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(0.f, context.windowWidth);
	std::uniform_real_distribution<float> positionY(0.f, context.windowHeight);
	std::uniform_int_distribution<int> edgeDist(0, 3);

	switch (edgeDist(rng))
	{
	case 0:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case 1:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case 2:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}

Asteroid CreateRandomAsteroid(const GameContext& context, std::mt19937& rng)
{
	const float radius = GetSatelliteRadius(context) * 2.f;
	const float spawnMargin = radius + 30.f * context.scale;

	std::uniform_real_distribution<float> speed(30.f, 60.f);
	std::uniform_real_distribution<float> direction(0.f, 6.2831853f);

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float asteroidSpeed = speed(rng) * context.scale;
	const float angle = direction(rng);
	const sf::Vector2f velocity(std::cos(angle) * asteroidSpeed, std::sin(angle) * asteroidSpeed);

	sf::CircleShape shape(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
	shape.setPosition(position);
	shape.setFillColor(kColorAsteroidFill);
	shape.setOutlineColor(kColorAsteroidOutline);
	shape.setOutlineThickness(1.f);

	return Asteroid{ shape, velocity };
}

std::vector<Asteroid> CreateInitialAsteroids(const GameContext& context, std::mt19937& rng)
{
	std::uniform_int_distribution<int> countDist(minAsteroidCount, maxAsteroidCount);
	const int asteroidCount = countDist(rng);

	std::vector<Asteroid> asteroids;
	asteroids.reserve(static_cast<std::size_t>(asteroidCount));
	for (int i = 0; i < asteroidCount; ++i)
	{
		asteroids.push_back(CreateRandomAsteroid(context, rng));
	}
	return asteroids;
}

sf::ConvexShape CreateNeutralTriangleShape(const GameContext& context)
{
	const sf::Vector2f size = GetNeutralShipSize(context);
	const float width = size.x;
	const float height = size.y;

	sf::ConvexShape shape(3);
	shape.setPoint(0, sf::Vector2f(width / 2.f, 0.f));
	shape.setPoint(1, sf::Vector2f(0.f, height));
	shape.setPoint(2, sf::Vector2f(width, height));
	shape.setOrigin(sf::Vector2f(width / 2.f, height / 2.f));
	shape.setFillColor(kColorNeutralFill);
	shape.setOutlineColor(kColorNeutralOutline);
	shape.setOutlineThickness(1.f);
	return shape;
}

sf::Vector2f CreateInboundVelocity(sf::Vector2f spawnPosition, const GameContext& context, float speed, std::mt19937& rng)
{
	const sf::Vector2f screenCenter(context.windowWidth / 2.f, context.windowHeight / 2.f);
	sf::Vector2f direction = screenCenter - spawnPosition;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.0001f)
	{
		direction /= length;
	}

	std::uniform_real_distribution<float> angleOffset(-0.45f, 0.45f);
	const float offsetAngle = angleOffset(rng);
	const float cosA = std::cos(offsetAngle);
	const float sinA = std::sin(offsetAngle);
	const sf::Vector2f rotated(
		direction.x * cosA - direction.y * sinA,
		direction.x * sinA + direction.y * cosA);

	return rotated * speed;
}

bool IsFullyOutsideScreen(const sf::FloatRect& bounds, const GameContext& context, float margin)
{
	return bounds.position.x + bounds.size.x < -margin
		|| bounds.position.x > context.windowWidth + margin
		|| bounds.position.y + bounds.size.y < -margin
		|| bounds.position.y > context.windowHeight + margin;
}

NeutralShip CreateRandomNeutralShip(const GameContext& context, std::mt19937& rng)
{
	const sf::Vector2f shipSize = GetNeutralShipSize(context);
	const float spawnMargin = std::max(shipSize.x, shipSize.y) + 40.f * context.scale;

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float speed = neutralMoveSpeed * context.scale;
	const sf::Vector2f velocity = CreateInboundVelocity(position, context, speed, rng);

	sf::ConvexShape shape = CreateNeutralTriangleShape(context);
	shape.setPosition(position);

	const float angleDegrees = std::atan2(velocity.y, velocity.x) * 180.f / 3.14159265f + 90.f;
	shape.setRotation(sf::degrees(angleDegrees));

	return NeutralShip{ shape, velocity };
}

std::vector<NeutralShip> CreateNeutralShips(const GameContext& context, std::mt19937& rng)
{
	std::vector<NeutralShip> neutrals;
	neutrals.reserve(neutralShipCount);
	for (int i = 0; i < neutralShipCount; ++i)
	{
		neutrals.push_back(CreateRandomNeutralShip(context, rng));
	}
	return neutrals;
}

void KeepInsideScreen(sf::Shape& shape, const GameContext& context, sf::Vector2f* velocity)
{
	sf::FloatRect bounds = shape.getGlobalBounds();

	if (bounds.position.x < 0.f)
	{
		shape.move(sf::Vector2f(-bounds.position.x, 0.f));
		if (velocity != nullptr)
		{
			velocity->x = std::abs(velocity->x);
		}
	}
	else if (bounds.position.x + bounds.size.x > context.windowWidth)
	{
		const float offset = context.windowWidth - (bounds.position.x + bounds.size.x);
		shape.move(sf::Vector2f(offset, 0.f));
		if (velocity != nullptr)
		{
			velocity->x = -std::abs(velocity->x);
		}
	}

	bounds = shape.getGlobalBounds();

	if (bounds.position.y < 0.f)
	{
		shape.move(sf::Vector2f(0.f, -bounds.position.y));
		if (velocity != nullptr)
		{
			velocity->y = std::abs(velocity->y);
		}
	}
	else if (bounds.position.y + bounds.size.y > context.windowHeight)
	{
		const float offset = context.windowHeight - (bounds.position.y + bounds.size.y);
		shape.move(sf::Vector2f(0.f, offset));
		if (velocity != nullptr)
		{
			velocity->y = -std::abs(velocity->y);
		}
	}
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

void UpdateAsteroids(std::vector<Asteroid>& asteroids, float deltaTime, const GameContext& context)
{
	for (auto& asteroid : asteroids)
	{
		asteroid.shape.move(asteroid.velocity * deltaTime);
		KeepInsideScreen(asteroid.shape, context, &asteroid.velocity);
	}
}

void UpdateNeutralShips(std::vector<NeutralShip>& neutrals, float deltaTime, const GameContext& context, std::mt19937& rng)
{
	const float removeMargin = 80.f * context.scale;

	for (auto& neutral : neutrals)
	{
		neutral.shape.move(neutral.velocity * deltaTime);
	}

	neutrals.erase(
		std::remove_if(
			neutrals.begin(),
			neutrals.end(),
			[&](const NeutralShip& neutral) {
				return IsFullyOutsideScreen(neutral.shape.getGlobalBounds(), context, removeMargin);
			}),
		neutrals.end());

	while (static_cast<int>(neutrals.size()) < neutralShipCount)
	{
		neutrals.push_back(CreateRandomNeutralShip(context, rng));
	}
}

void UpdateBullets(std::vector<Bullet>& bullets, float deltaTime, const GameContext& context)
{
	const float removeMargin = 40.f * context.scale;

	for (auto& bullet : bullets)
	{
		bullet.shape.move(bullet.velocity * deltaTime);
	}

	bullets.erase(
		std::remove_if(
			bullets.begin(),
			bullets.end(),
			[&](const Bullet& bullet) {
				const sf::Vector2f position = bullet.shape.getPosition();
				return position.x < -removeMargin
					|| position.x > context.windowWidth + removeMargin
					|| position.y < -removeMargin
					|| position.y > context.windowHeight + removeMargin;
			}),
		bullets.end());
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

	const sf::Vector2f earthCenter = GetEarthCenter(context);

	sf::CircleShape earth = CreateEarth(context);
	std::vector<OrbitSatellite> orbitSatellites = CreateOrbitSatellites(context);
	sf::ConvexShape playerShip = CreatePlayerShip(context);
	playerShip.setPosition(sf::Vector2f(
		context.windowWidth / 2.f - context.shipWidth / 2.f,
		context.windowHeight / 2.f - context.shipHeight / 2.f));

	std::mt19937 rng(std::random_device{}());
	std::vector<Asteroid> asteroids = CreateInitialAsteroids(context, rng);
	std::vector<NeutralShip> neutralShips = CreateNeutralShips(context, rng);
	std::vector<Bullet> bullets;
	bullets.reserve(64);

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
				else if (keyPressed->code == sf::Keyboard::Key::Space)
				{
					bullets.push_back(CreateBulletFromShip(playerShip, context));
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

		playerShip.move(offset);
		KeepInsideScreen(playerShip, context, nullptr);
		UpdateShipRotation(playerShip, offset);

		UpdateAsteroids(asteroids, deltaTime, context);
		UpdateNeutralShips(neutralShips, deltaTime, context, rng);
		UpdateBullets(bullets, deltaTime, context);
		UpdateOrbitSatellites(orbitSatellites, deltaTime, earthCenter, context);

		window.clear(sf::Color(10, 10, 25));
		window.draw(earth);
		for (const auto& satellite : orbitSatellites)
		{
			window.draw(satellite.shape);
			DrawSatelliteHpBar(window, satellite, context);
		}
		for (const auto& asteroid : asteroids)
		{
			window.draw(asteroid.shape);
		}
		for (const auto& neutral : neutralShips)
		{
			window.draw(neutral.shape);
		}
		for (const auto& bullet : bullets)
		{
			window.draw(bullet.shape);
		}
		window.draw(playerShip);
		window.display();
	}

	return 0;
}
