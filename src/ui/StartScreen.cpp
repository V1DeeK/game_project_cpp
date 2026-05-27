#include "ui/StartScreen.hpp"

#include <cmath>
#include <optional>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"
#include "systems/SatelliteOrbitSystem.hpp"

namespace
{
sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window)
{
	const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
	return window.mapPixelToCoords(mousePixel);
}

bool IsMouseOver(const sf::RectangleShape& rect, sf::Vector2f point)
{
	return rect.getGlobalBounds().contains(point);
}

sf::ConvexShape CreatePlayIcon(float size)
{
	sf::ConvexShape icon(3);
	icon.setPoint(0, sf::Vector2f(-size * 0.35f, -size * 0.4f));
	icon.setPoint(1, sf::Vector2f(-size * 0.35f, size * 0.4f));
	icon.setPoint(2, sf::Vector2f(size * 0.45f, game::screenOrigin));
	icon.setFillColor(sf::Color::White);
	const sf::FloatRect bounds = icon.getLocalBounds();
	icon.setOrigin(sf::Vector2f(
		bounds.position.x + bounds.size.x * game::half,
		bounds.position.y + bounds.size.y * game::half));
	return icon;
}

sf::VertexArray CreateXIcon(float size)
{
	// 2 lines, 4 vertices, rendered as Lines.
	sf::VertexArray icon(sf::PrimitiveType::Lines, 4);
	const float r = size * 0.35f;
	icon[0].position = sf::Vector2f(-r, -r);
	icon[1].position = sf::Vector2f(r, r);
	icon[2].position = sf::Vector2f(-r, r);
	icon[3].position = sf::Vector2f(r, -r);
	for (std::size_t i = 0; i < 4; ++i)
	{
		icon[i].color = sf::Color::White;
	}
	return icon;
}
} // namespace

namespace game
{
StartScreen::StartScreen(const GameContext& context, std::mt19937& rng)
	: m_earth(Earth::Create(context))
	, m_earthCenter(m_earth.GetCenter())
	, m_satellites(SatelliteOrbitSystem::CreateOrbitSatellites(context))
	, m_asteroids(Asteroid::CreateInitialFleet(context, rng))
{
	UpdateLayout(context);
}

StartScreenAction StartScreen::ProcessEvent(sf::RenderWindow& window, const sf::Event& event)
{
	if (event.is<sf::Event::Closed>())
	{
		return StartScreenAction::Exit;
	}

	if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
	{
		if (keyPressed->code == sf::Keyboard::Key::Escape)
		{
			return StartScreenAction::Exit;
		}
		if (keyPressed->code == sf::Keyboard::Key::Enter)
		{
			return StartScreenAction::StartGame;
		}
	}

	if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		window.requestFocus();
		if (mousePressed->button == sf::Mouse::Button::Left)
		{
			const sf::Vector2f mouseWorld = GetMouseWorldPosition(window);
			if (IsMouseOver(m_startButton, mouseWorld))
			{
				return StartScreenAction::StartGame;
			}
			if (IsMouseOver(m_exitButton, mouseWorld))
			{
				return StartScreenAction::Exit;
			}
		}
	}

	if (event.is<sf::Event::FocusGained>())
	{
		window.requestFocus();
	}

	return StartScreenAction::None;
}

void StartScreen::Update(float deltaTime, const GameContext& context, std::mt19937& rng)
{
	m_earthCenter = m_earth.GetCenter();
	SatelliteOrbitSystem::UpdateOrbitSatellites(m_satellites, deltaTime, m_earthCenter, context);
	Asteroid::UpdateAll(m_asteroids, deltaTime, context, rng);
}

void StartScreen::Render(sf::RenderWindow& window, const GameContext& context) const
{
	(void)context;

	const sf::Vector2f mouseWorld = GetMouseWorldPosition(window);

	const bool startHover = IsMouseOver(m_startButton, mouseWorld);
	const bool exitHover = IsMouseOver(m_exitButton, mouseWorld);

	sf::RectangleShape startBtn = m_startButton;
	sf::RectangleShape exitBtn = m_exitButton;
	startBtn.setFillColor(startHover ? sf::Color(60, 150, 70) : sf::Color(40, 120, 55));
	exitBtn.setFillColor(exitHover ? sf::Color(170, 70, 70) : sf::Color(140, 55, 55));

	window.clear(kColorBackgroundClear);
	m_earth.Draw(window);
	for (const auto& satellite : m_satellites)
	{
		satellite.Draw(window);
	}
	for (const auto& asteroid : m_asteroids)
	{
		asteroid.Draw(window);
	}

	window.draw(startBtn);
	window.draw(exitBtn);

	sf::ConvexShape playIcon = m_startIcon;
	// Button position is already its center (origin is set in UpdateLayout).
	playIcon.setPosition(startBtn.getPosition());
	window.draw(playIcon);

	sf::VertexArray xIcon = CreateXIcon(std::min(exitBtn.getSize().x, exitBtn.getSize().y));
	const sf::Vector2f exitButtonCenter = exitBtn.getPosition();
	for (std::size_t i = 0; i < xIcon.getVertexCount(); ++i)
	{
		xIcon[i].position += exitButtonCenter;
	}
	window.draw(xIcon);
}

void StartScreen::UpdateLayout(const GameContext& context)
{
	const float buttonWidth = 240.f * context.scale;
	const float buttonHeight = 70.f * context.scale;
	const float gap = 20.f * context.scale;
	const sf::Vector2f center(context.windowWidth * half, context.windowHeight * half);

	m_startButton = sf::RectangleShape(sf::Vector2f(buttonWidth, buttonHeight));
	m_startButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_startButton.setPosition(sf::Vector2f(center.x, center.y + 120.f * context.scale));
	m_startButton.setOutlineColor(sf::Color(255, 255, 255, 70));
	m_startButton.setOutlineThickness(defaultOutlineThickness);

	m_exitButton = sf::RectangleShape(sf::Vector2f(buttonWidth, buttonHeight));
	m_exitButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_exitButton.setPosition(sf::Vector2f(center.x, m_startButton.getPosition().y + buttonHeight + gap));
	m_exitButton.setOutlineColor(sf::Color(255, 255, 255, 70));
	m_exitButton.setOutlineThickness(defaultOutlineThickness);

	m_startIcon = CreatePlayIcon(std::min(buttonWidth, buttonHeight));
}
} // namespace game

