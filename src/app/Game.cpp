#include "app/Game.hpp"

#include <algorithm>
#include <optional>

#include <SFML/Window/VideoMode.hpp>

#include "collision/CollisionSystem.hpp"
#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/SatelliteOrbitSystem.hpp"

namespace game
{
Game::Game()
	: m_context(CreateGameContext(sf::VideoMode::getDesktopMode()))
	, m_window(sf::VideoMode::getDesktopMode(), "GAME", sf::State::Fullscreen)
	, m_rng(std::random_device{}())
	, m_startScreen(m_context, m_rng)
	, m_earth(Earth::Create(m_context))
	, m_orbitSatellites(SatelliteOrbitSystem::CreateOrbitSatellites(m_context))
	, m_playerShip(PlayerShip::CreateAtCenter(m_context))
	, m_asteroids(Asteroid::CreateInitialFleet(m_context, m_rng))
	, m_neutralShips(NeutralShip::CreateInitialFleet(m_context, m_rng))
{
	m_window.setFramerateLimit(targetFrameRate);
	(void)m_window.setActive(true);
	m_window.requestFocus();

	m_earthCenter = m_earth.GetCenter();
	m_enemies.reserve(static_cast<std::size_t>(initialEnemyCapacity));
	m_bullets.reserve(static_cast<std::size_t>(initialBulletCapacity));
}

void Game::Run()
{
	while (m_window.isOpen())
	{
		float deltaTime = m_clock.restart().asSeconds();
		deltaTime = std::clamp(deltaTime, minDeltaTime, maxDeltaTime);

		ProcessEvents();
		Update(deltaTime);
		Render();
	}
}

void Game::ProcessEvents()
{
	while (const std::optional event = m_window.pollEvent())
	{
		if (m_mode == Mode::StartScreen)
		{
			const StartScreenAction action = m_startScreen.ProcessEvent(m_window, *event);
			if (action == StartScreenAction::Exit)
			{
				m_window.close();
			}
			else if (action == StartScreenAction::StartGame)
			{
				ResetPlayingState();
				m_recordStore.ResetCurrentRun();
				m_mode = Mode::Playing;
			}
			continue;
		}

		if (m_playerInput.ProcessEvent(m_window, *event, m_playerShip, m_bullets, m_context))
		{
			m_window.close();
		}
	}
}

void Game::Update(float deltaTime)
{
	if (m_mode == Mode::StartScreen)
	{
		m_startScreen.Update(deltaTime, m_context, m_rng);
		return;
	}

	m_playerInput.UpdatePlayer(m_playerShip, m_window, deltaTime, m_context);
	m_recordStore.Update(deltaTime);

	Asteroid::UpdateAll(m_asteroids, deltaTime, m_context, m_rng);
	Asteroid::ProcessMerges(m_asteroids);
	NeutralShip::UpdateAll(m_neutralShips, deltaTime, m_context, m_rng);
	SatelliteOrbitSystem::UpdateOrbitSatellites(m_orbitSatellites, deltaTime, m_earthCenter, m_context);
	EnemyAISystem::UpdateAll(
		m_enemies,
		m_bullets,
		m_orbitSatellites,
		deltaTime,
		m_earthCenter,
		m_playerShip.GetPosition(),
		m_context);
	Bullet::UpdateAll(m_bullets, deltaTime, m_context);
	if (CollisionSystem::ProcessFrame(
			m_playerShip,
			m_asteroids,
			m_enemies,
			m_neutralShips,
			m_orbitSatellites,
			m_bullets,
			m_context,
			m_earthCenter,
			m_rng))
	{
		m_recordStore.CommitCurrentRun();
		m_window.close();
	}
}

void Game::Render()
{
	if (m_mode == Mode::StartScreen)
	{
		m_startScreen.Render(m_window, m_context);
		m_recordDisplay.RenderRecord(m_window, m_context, m_recordStore.GetHighScore());
		m_window.display();
		return;
	}

	m_window.clear(kColorBackgroundClear);
	m_earth.Draw(m_window);
	for (const auto& satellite : m_orbitSatellites)
	{
		satellite.Draw(m_window);
		satellite.DrawHpBar(m_window, m_context);
	}
	for (const auto& asteroid : m_asteroids)
	{
		asteroid.Draw(m_window);
	}
	for (const auto& neutral : m_neutralShips)
	{
		neutral.Draw(m_window);
	}
	for (const auto& enemy : m_enemies)
	{
		enemy.Draw(m_window);
	}
	for (const auto& bullet : m_bullets)
	{
		bullet.Draw(m_window);
	}
	m_playerShip.Draw(m_window);
	m_recordDisplay.RenderScore(m_window, m_context, m_recordStore.GetCurrentScore());
	m_window.display();
}

void Game::ResetPlayingState()
{
	m_earth = Earth::Create(m_context);
	m_earthCenter = m_earth.GetCenter();
	m_orbitSatellites = SatelliteOrbitSystem::CreateOrbitSatellites(m_context);
	m_playerShip = PlayerShip::CreateAtCenter(m_context);

	m_asteroids = Asteroid::CreateInitialFleet(m_context, m_rng);
	m_neutralShips = NeutralShip::CreateInitialFleet(m_context, m_rng);
	m_enemies.clear();
	m_bullets.clear();
	m_enemies.reserve(static_cast<std::size_t>(initialEnemyCapacity));
	m_bullets.reserve(static_cast<std::size_t>(initialBulletCapacity));
}
} // namespace game
