#pragma once

#include <SFML/Graphics/ConvexShape.hpp>

#include "app/GameContext.hpp"
#include "entities/Bullet.hpp"

namespace game
{
class Ship;

class FiringSystem
{
public:
	static Bullet CreateBulletFromShip(const Ship& ship, const GameContext& context);
	static Bullet CreateBulletFromShip(const sf::ConvexShape& shipShape, const GameContext& context);

private:
	static Bullet CreateBulletFromShipShape(const sf::ConvexShape& shipShape, const GameContext& context);
};
} // namespace game
