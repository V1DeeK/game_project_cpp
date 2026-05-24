#include "entities/Bullet.hpp"

#include <algorithm>
#include <cmath>

#include "config/Colors.hpp"

namespace
{
sf::Vector2f GetShipForwardDirection(const sf::ConvexShape& ship, const game::GameContext& context)
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

sf::Vector2f GetShipNosePosition(const sf::ConvexShape& ship, const game::GameContext& context)
{
	return ship.getTransform().transformPoint(sf::Vector2f(context.shipWidth / 2.f, 0.f));
}
} // namespace

namespace game
{
Bullet Bullet::CreateFromShip(const sf::ConvexShape& ship, const GameContext& context)
{
	const sf::Vector2f direction = GetShipForwardDirection(ship, context);
	const sf::Vector2f nosePosition = GetShipNosePosition(ship, context);

	const float bulletWidth = 2.f * context.scale;
	const float bulletLength = 5.f * context.scale;

	Bullet bullet;
	bullet.m_shape = sf::RectangleShape(sf::Vector2f(bulletWidth, bulletLength));
	bullet.m_shape.setOrigin(sf::Vector2f(bulletWidth / 2.f, bulletLength / 2.f));
	bullet.m_shape.setPosition(nosePosition + direction * (bulletLength * 0.6f));
	bullet.m_shape.setFillColor(kColorBullet);

	const float angleDegrees = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f + 90.f;
	bullet.m_shape.setRotation(sf::degrees(angleDegrees));
	bullet.m_velocity = direction * context.bulletSpeed;

	return bullet;
}

void Bullet::UpdateAll(std::vector<Bullet>& bullets, float deltaTime, const GameContext& context)
{
	for (auto& bullet : bullets)
	{
		bullet.Update(deltaTime);
	}

	RemoveOffScreen(bullets, context);
}

void Bullet::RemoveOffScreen(std::vector<Bullet>& bullets, const GameContext& context)
{
	bullets.erase(
		std::remove_if(
			bullets.begin(),
			bullets.end(),
			[&](const Bullet& bullet) { return bullet.IsOffScreen(context); }),
		bullets.end());
}

void Bullet::Update(float deltaTime)
{
	m_shape.move(m_velocity * deltaTime);
}

bool Bullet::IsOffScreen(const GameContext& context) const
{
	const float removeMargin = 40.f * context.scale;
	const sf::Vector2f position = m_shape.getPosition();
	return position.x < -removeMargin
		|| position.x > context.windowWidth + removeMargin
		|| position.y < -removeMargin
		|| position.y > context.windowHeight + removeMargin;
}

void Bullet::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

sf::Vector2f Bullet::GetCenter() const
{
	const sf::FloatRect bounds = m_shape.getGlobalBounds();
	return sf::Vector2f(
		bounds.position.x + bounds.size.x / 2.f,
		bounds.position.y + bounds.size.y / 2.f);
}

sf::FloatRect Bullet::GetBounds() const
{
	return m_shape.getGlobalBounds();
}

bool Bullet::IntersectsConvexShape(const sf::ConvexShape& shape) const
{
	return shape.getGlobalBounds().contains(GetCenter());
}
} 
