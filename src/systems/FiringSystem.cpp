#include "systems/FiringSystem.hpp"

#include <cmath>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "entities/Ship.hpp"

namespace game
{
Bullet FiringSystem::CreateBulletFromShip(const Ship& ship, const GameContext& context)
{
	return CreateBulletFromShipShape(ship.GetShape(), context);
}

Bullet FiringSystem::CreateBulletFromShip(const sf::ConvexShape& shipShape, const GameContext& context)
{
	return CreateBulletFromShipShape(shipShape, context);
}

Bullet FiringSystem::CreateBulletFromShipShape(const sf::ConvexShape& shipShape, const GameContext& context)
{
	const sf::Vector2f direction = Ship::GetForwardDirection(shipShape, context);
	const sf::Vector2f nosePosition = Ship::GetNosePosition(shipShape, context);

	const float bulletWidth = bulletWidthBase * context.scale;
	const float bulletLength = bulletLengthBase * context.scale;

	Bullet bullet;
	bullet.m_shape = sf::RectangleShape(sf::Vector2f(bulletWidth, bulletLength));
	bullet.m_shape.setOrigin(sf::Vector2f(bulletWidth * half, bulletLength * half));
	bullet.m_shape.setPosition(nosePosition + direction * (bulletLength * bulletMuzzleOffsetFactor));
	bullet.m_shape.setFillColor(kColorBullet);

	const float angleDegrees = std::atan2(direction.y, direction.x) * degreesPerRadian + shipRotationOffsetDegrees;
	bullet.m_shape.setRotation(sf::degrees(angleDegrees));
	bullet.m_velocity = direction * context.bulletSpeed;

	return bullet;
}
} // namespace game
