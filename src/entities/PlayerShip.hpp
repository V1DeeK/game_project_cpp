#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include "app/GameContext.hpp"
#include "entities/Ship.hpp"

namespace game
{
class PlayerShip : public Ship
{
public:
	static PlayerShip CreateAtCenter(const GameContext& context);

	void UpdateAim(const sf::RenderWindow& window);
	void ApplyMovement(bool moveLeft, bool moveRight, bool moveUp, bool moveDown, float deltaTime, const GameContext& context);
	void ClampToScreen(const GameContext& context);

private:
	explicit PlayerShip(sf::ConvexShape shape);
};
} // namespace game
