#include "ui/StartScreen.hpp"

#include <cmath>
#include <optional>
#include <vector>

#include <SFML/Graphics/Color.hpp>
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

bool IsMouseOver(const sf::Shape& shape, sf::Vector2f point)
{
	return shape.getGlobalBounds().contains(point);
}

sf::ConvexShape CreateRoundedRectangle(sf::Vector2f size, float cornerRadius, int cornerSegments)
{
	const float width = size.x;
	const float height = size.y;
	const float radius = std::min(cornerRadius, std::min(width, height) * game::half);

	std::vector<sf::Vector2f> points;
	points.reserve(static_cast<std::size_t>(cornerSegments) * 4 + 4);

	const auto addArc = [&](float centerX, float centerY, float startAngle, float endAngle, bool skipFirstPoint) {
		for (int segmentIndex = (skipFirstPoint ? 1 : 0); segmentIndex <= cornerSegments; ++segmentIndex)
		{
			const float angle = startAngle
				+ (endAngle - startAngle) * static_cast<float>(segmentIndex) / static_cast<float>(cornerSegments);
			points.emplace_back(
				centerX + std::cos(angle) * radius,
				centerY + std::sin(angle) * radius);
		}
	};

	addArc(radius, radius, game::pi, game::pi + game::half * game::pi, false);
	addArc(width - radius, radius, game::pi + game::half * game::pi, game::twoPi, true);
	addArc(width - radius, height - radius, game::screenOrigin, game::half * game::pi, true);
	addArc(radius, height - radius, game::half * game::pi, game::pi, true);

	sf::ConvexShape shape(points.size());
	for (std::size_t pointIndex = 0; pointIndex < points.size(); ++pointIndex)
	{
		shape.setPoint(static_cast<std::size_t>(pointIndex), points[pointIndex]);
	}

	return shape;
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
	m_fontReady = LoadFont();
	if (m_fontReady)
	{
		const unsigned int fontSize = static_cast<unsigned int>(startScreenButtonFontSizeBase * context.scale);
		m_startLabel.emplace(m_font, "START", fontSize);
		m_exitLabel.emplace(m_font, "EXIT", fontSize);

		for (sf::Text* label : {&*m_startLabel, &*m_exitLabel})
		{
			label->setFillColor(sf::Color::White);
			label->setOutlineColor(sf::Color(0, 0, 0, 160));
			label->setOutlineThickness(defaultOutlineThickness);
		}
	}

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

	sf::ConvexShape startBtn = m_startButton;
	sf::ConvexShape exitBtn = m_exitButton;
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

	if (m_fontReady && m_startLabel.has_value() && m_exitLabel.has_value())
	{
		sf::Text startLabel = *m_startLabel;
		sf::Text exitLabel = *m_exitLabel;
		CenterLabelOnButton(startLabel, startBtn);
		CenterLabelOnButton(exitLabel, exitBtn);
		window.draw(startLabel);
		window.draw(exitLabel);
	}
}

bool StartScreen::LoadFont()
{
	const char* fontPaths[] = {
		"/System/Library/Fonts/Supplemental/Arial.ttf",
		"/Library/Fonts/Arial.ttf",
		"assets/fonts/Arial.ttf",
	};

	for (const char* path : fontPaths)
	{
		if (m_font.openFromFile(path))
		{
			return true;
		}
	}

	return false;
}

void StartScreen::UpdateLayout(const GameContext& context)
{
	const float buttonWidth = startScreenButtonWidthBase * context.scale;
	const float buttonHeight = startScreenButtonHeightBase * context.scale;
	const float cornerRadius = startScreenButtonCornerRadiusBase * context.scale;
	const float gap = startScreenButtonGapBase * context.scale;
	const sf::Vector2f center(context.windowWidth * half, context.windowHeight * half);

	const sf::Vector2f buttonSize(buttonWidth, buttonHeight);
	m_startButton = CreateRoundedRectangle(buttonSize, cornerRadius, startScreenButtonCornerSegments);
	m_startButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_startButton.setPosition(sf::Vector2f(
		center.x,
		center.y + startScreenButtonOffsetYBase * context.scale + startScreenButtonOffsetYExtra));
	m_startButton.setOutlineColor(sf::Color(255, 255, 255, 70));
	m_startButton.setOutlineThickness(defaultOutlineThickness);

	m_exitButton = CreateRoundedRectangle(buttonSize, cornerRadius, startScreenButtonCornerSegments);
	m_exitButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_exitButton.setPosition(sf::Vector2f(center.x, m_startButton.getPosition().y + buttonHeight + gap));
	m_exitButton.setOutlineColor(sf::Color(255, 255, 255, 70));
	m_exitButton.setOutlineThickness(defaultOutlineThickness);
}

void StartScreen::CenterLabelOnButton(sf::Text& label, const sf::Shape& button) const
{
	const sf::FloatRect bounds = label.getLocalBounds();
	label.setOrigin(sf::Vector2f(
		bounds.position.x + bounds.size.x * half,
		bounds.position.y + bounds.size.y * half));
	label.setPosition(button.getPosition());
}
} // namespace game
