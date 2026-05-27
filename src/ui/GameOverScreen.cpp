#include "ui/GameOverScreen.hpp"

#include <cmath>
#include <string>
#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <cstdint>

#include "config/Colors.hpp"
#include "config/GameConstants.hpp"

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
	points.reserve(static_cast<std::size_t>(cornerSegments) * game::roundedRectangleCornerCount + game::roundedRectangleExtraPoints);

	const auto addArc = [&](float centerX, float centerY, float startAngle, float endAngle, bool skipFirstPoint) {
		for (int segmentIndex = (skipFirstPoint ? game::indexIncrement : game::hitPointsDepleted); segmentIndex <= cornerSegments;
			 ++segmentIndex)
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
		shape.setPoint(pointIndex, points[pointIndex]);
	}

	return shape;
}
} // namespace

namespace game
{
GameOverScreen::GameOverScreen(const GameContext& context)
{
	m_fontReady = LoadFont();
	if (m_fontReady)
	{
		const unsigned int scoreFontSize = static_cast<unsigned int>(gameOverScoreFontSizeBase * context.scale);
		const unsigned int buttonFontSize = static_cast<unsigned int>(gameOverButtonFontSizeBase * context.scale);

		m_recordLabel.emplace(m_font, "", scoreFontSize);
		m_scoreLabel.emplace(m_font, "", scoreFontSize);
		m_restartLabel.emplace(m_font, "RESTART", buttonFontSize);
		m_exitLabel.emplace(m_font, "EXIT", buttonFontSize);

		for (sf::Text* label : {&*m_recordLabel, &*m_scoreLabel, &*m_restartLabel, &*m_exitLabel})
		{
			label->setFillColor(sf::Color::White);
			label->setOutlineColor(kColorUiTextOutline);
			label->setOutlineThickness(defaultOutlineThickness);
		}
	}

	UpdateLayout(context);
}

void GameOverScreen::Show(int record, int currentScore)
{
	m_record = record;
	m_currentScore = currentScore;
}

GameOverAction GameOverScreen::ProcessEvent(sf::RenderWindow& window, const sf::Event& event)
{
	if (event.is<sf::Event::Closed>())
	{
		return GameOverAction::Exit;
	}

	if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
	{
		if (keyPressed->code == sf::Keyboard::Key::Escape)
		{
			return GameOverAction::Exit;
		}
		if (keyPressed->code == sf::Keyboard::Key::Enter)
		{
			return GameOverAction::Restart;
		}
	}

	if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		window.requestFocus();
		if (mousePressed->button == sf::Mouse::Button::Left)
		{
			const sf::Vector2f mouseWorld = GetMouseWorldPosition(window);
			if (IsMouseOver(m_restartButton, mouseWorld))
			{
				return GameOverAction::Restart;
			}
			if (IsMouseOver(m_exitButton, mouseWorld))
			{
				return GameOverAction::Exit;
			}
		}
	}

	return GameOverAction::None;
}

void GameOverScreen::Render(sf::RenderWindow& window, const GameContext& context) const
{
	if (m_fontReady && m_recordLabel.has_value() && m_scoreLabel.has_value())
	{
		m_recordLabel->setString("Record: " + std::to_string(m_record));
		m_scoreLabel->setString("Score: " + std::to_string(m_currentScore));

		const sf::Vector2f panelCenter = m_panel.getPosition();
		const float lineSpacing = gameOverScoreLineSpacingBase * context.scale;

		const sf::FloatRect recordBounds = m_recordLabel->getLocalBounds();
		m_recordLabel->setOrigin(sf::Vector2f(
			recordBounds.position.x + recordBounds.size.x * half,
			recordBounds.position.y + recordBounds.size.y * half));
		m_recordLabel->setPosition(sf::Vector2f(panelCenter.x, panelCenter.y - lineSpacing));

		const sf::FloatRect scoreBounds = m_scoreLabel->getLocalBounds();
		m_scoreLabel->setOrigin(sf::Vector2f(
			scoreBounds.position.x + scoreBounds.size.x * half,
			scoreBounds.position.y + scoreBounds.size.y * half));
		m_scoreLabel->setPosition(sf::Vector2f(panelCenter.x, panelCenter.y - lineSpacing * half));
	}

	const sf::Vector2f mouseWorld = GetMouseWorldPosition(window);
	const bool restartHover = IsMouseOver(m_restartButton, mouseWorld);
	const bool exitHover = IsMouseOver(m_exitButton, mouseWorld);

	sf::ConvexShape restartBtn = m_restartButton;
	sf::ConvexShape exitBtn = m_exitButton;
	restartBtn.setFillColor(restartHover ? kColorUiStartButtonHover : kColorUiStartButton);
	exitBtn.setFillColor(exitHover ? kColorUiExitButtonHover : kColorUiExitButton);

	window.draw(m_overlay);
	window.draw(m_panel);
	window.draw(restartBtn);
	window.draw(exitBtn);

	if (m_fontReady && m_recordLabel.has_value() && m_scoreLabel.has_value() && m_restartLabel.has_value()
		&& m_exitLabel.has_value())
	{
		sf::Text restartLabel = *m_restartLabel;
		sf::Text exitLabel = *m_exitLabel;
		CenterLabelOnButton(restartLabel, restartBtn);
		CenterLabelOnButton(exitLabel, exitBtn);
		window.draw(*m_recordLabel);
		window.draw(*m_scoreLabel);
		window.draw(restartLabel);
		window.draw(exitLabel);
	}
}

bool GameOverScreen::LoadFont()
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

void GameOverScreen::UpdateLayout(const GameContext& context)
{
	m_overlay = sf::RectangleShape(sf::Vector2f(context.windowWidth, context.windowHeight));
	m_overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(gameOverOverlayAlpha)));

	const float panelWidth = gameOverPanelWidthBase * context.scale;
	const float panelHeight = gameOverPanelHeightBase * context.scale;
	const float cornerRadius = gameOverPanelCornerRadiusBase * context.scale;
	const sf::Vector2f center(context.windowWidth * half, context.windowHeight * half);

	m_panel = CreateRoundedRectangle(sf::Vector2f(panelWidth, panelHeight), cornerRadius, startScreenButtonCornerSegments);
	m_panel.setOrigin(sf::Vector2f(panelWidth * half, panelHeight * half));
	m_panel.setPosition(center);
	m_panel.setFillColor(kColorUiGameOverPanelFill);
	m_panel.setOutlineColor(kColorUiGameOverPanelOutline);
	m_panel.setOutlineThickness(defaultOutlineThickness);

	const float buttonWidth = gameOverButtonWidthBase * context.scale;
	const float buttonHeight = gameOverButtonHeightBase * context.scale;
	const float buttonCornerRadius = gameOverButtonCornerRadiusBase * context.scale;
	const float buttonGap = gameOverButtonGapBase * context.scale;
	const sf::Vector2f buttonSize(buttonWidth, buttonHeight);

	m_restartButton = CreateRoundedRectangle(buttonSize, buttonCornerRadius, startScreenButtonCornerSegments);
	m_restartButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_restartButton.setPosition(sf::Vector2f(center.x, center.y + gameOverButtonOffsetYBase * context.scale));
	m_restartButton.setOutlineColor(kColorUiButtonOutline);
	m_restartButton.setOutlineThickness(defaultOutlineThickness);

	m_exitButton = CreateRoundedRectangle(buttonSize, buttonCornerRadius, startScreenButtonCornerSegments);
	m_exitButton.setOrigin(sf::Vector2f(buttonWidth * half, buttonHeight * half));
	m_exitButton.setPosition(sf::Vector2f(center.x, m_restartButton.getPosition().y + buttonHeight + buttonGap));
	m_exitButton.setOutlineColor(kColorUiButtonOutline);
	m_exitButton.setOutlineThickness(defaultOutlineThickness);
}

void GameOverScreen::CenterLabelOnButton(sf::Text& label, const sf::Shape& button) const
{
	const sf::FloatRect bounds = label.getLocalBounds();
	label.setOrigin(sf::Vector2f(
		bounds.position.x + bounds.size.x * half,
		bounds.position.y + bounds.size.y * half));
	label.setPosition(button.getPosition());
}
} // namespace game
