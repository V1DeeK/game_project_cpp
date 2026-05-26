#pragma once

#include <vector>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "app/GameContext.hpp"
#include "entities/Bullet.hpp"
#include "entities/PlayerShip.hpp"

namespace game
{
class PlayerInput
{
public:
	bool ProcessEvent(
		sf::RenderWindow& window,
		const sf::Event& event,
		PlayerShip& playerShip,
		std::vector<Bullet>& bullets,
		const GameContext& context);

	void UpdatePlayer(PlayerShip& playerShip, sf::RenderWindow& window, float deltaTime, const GameContext& context);

private:
	void ResetMovement();

	bool m_moveLeft = false;
	bool m_moveRight = false;
	bool m_moveUp = false;
	bool m_moveDown = false;
};
} // namespace game
