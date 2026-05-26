#include "input/PlayerInput.hpp"

#include "systems/FiringSystem.hpp"

namespace game
{
bool PlayerInput::ProcessEvent(
	sf::RenderWindow& window,
	const sf::Event& event,
	PlayerShip& playerShip,
	std::vector<Bullet>& bullets,
	const GameContext& context)
{
	if (event.is<sf::Event::Closed>())
	{
		return true;
	}

	if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		window.requestFocus();
		if (mousePressed->button == sf::Mouse::Button::Left)
		{
			playerShip.UpdateAim(window);
			bullets.push_back(FiringSystem::CreateBulletFromShip(playerShip, context));
		}
	}
	else if (event.is<sf::Event::FocusGained>())
	{
		window.requestFocus();
	}
	else if (event.is<sf::Event::FocusLost>())
	{
		ResetMovement();
	}
	else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
	{
		if (keyPressed->code == sf::Keyboard::Key::Escape)
		{
			return true;
		}

		switch (keyPressed->scancode)
		{
		case sf::Keyboard::Scancode::A:
			m_moveLeft = true;
			break;
		case sf::Keyboard::Scancode::D:
			m_moveRight = true;
			break;
		case sf::Keyboard::Scancode::W:
			m_moveUp = true;
			break;
		case sf::Keyboard::Scancode::S:
			m_moveDown = true;
			break;
		default:
			break;
		}
	}
	else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
	{
		switch (keyReleased->scancode)
		{
		case sf::Keyboard::Scancode::A:
			m_moveLeft = false;
			break;
		case sf::Keyboard::Scancode::D:
			m_moveRight = false;
			break;
		case sf::Keyboard::Scancode::W:
			m_moveUp = false;
			break;
		case sf::Keyboard::Scancode::S:
			m_moveDown = false;
			break;
		default:
			break;
		}
	}

	return false;
}

void PlayerInput::UpdatePlayer(
	PlayerShip& playerShip,
	sf::RenderWindow& window,
	float deltaTime,
	const GameContext& context)
{
	playerShip.ApplyMovement(m_moveLeft, m_moveRight, m_moveUp, m_moveDown, deltaTime, context);
	playerShip.ClampToScreen(context);
	playerShip.UpdateAim(window);
}

void PlayerInput::ResetMovement()
{
	m_moveLeft = false;
	m_moveRight = false;
	m_moveUp = false;
	m_moveDown = false;
}
} // namespace game
