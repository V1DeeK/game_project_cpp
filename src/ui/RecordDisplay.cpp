#include "ui/RecordDisplay.hpp"

#include <string>

#include "config/GameConstants.hpp"

namespace
{
bool TryOpenFont(sf::Font& font, const char* path)
{
	return font.openFromFile(path);
}
} // namespace

namespace game
{
RecordDisplay::RecordDisplay()
{
	if (!LoadFont())
	{
		return;
	}

	m_text.emplace(m_font, "", static_cast<unsigned int>(recordHudFontSizeBase));
	m_text->setFillColor(sf::Color::White);
	m_text->setOutlineColor(sf::Color(0, 0, 0, 160));
	m_text->setOutlineThickness(defaultOutlineThickness);
	m_ready = true;
}

void RecordDisplay::RenderRecord(sf::RenderWindow& window, const GameContext& context, int highScore) const
{
	if (!m_ready || !m_text.has_value())
	{
		return;
	}

	UpdateLayout(context, "Record: ", highScore);
	window.draw(*m_text);
}

void RecordDisplay::RenderScore(sf::RenderWindow& window, const GameContext& context, int currentScore) const
{
	if (!m_ready || !m_text.has_value())
	{
		return;
	}

	UpdateLayout(context, "Score: ", currentScore);
	window.draw(*m_text);
}

bool RecordDisplay::LoadFont()
{
	const char* fontPaths[] = {
		"/System/Library/Fonts/Supplemental/Arial.ttf",
		"/Library/Fonts/Arial.ttf",
		"assets/fonts/Arial.ttf",
	};

	for (const char* path : fontPaths)
	{
		if (TryOpenFont(m_font, path))
		{
			return true;
		}
	}

	return false;
}

void RecordDisplay::UpdateLayout(const GameContext& context, const char* label, int value) const
{
	const unsigned int characterSize = static_cast<unsigned int>(recordHudFontSizeBase * context.scale);
	m_text->setCharacterSize(characterSize);
	m_text->setString(std::string(label) + std::to_string(value));

	const sf::FloatRect bounds = m_text->getLocalBounds();
	const float marginX = recordHudMarginX * context.scale;
	const float marginY = recordHudMarginY * context.scale;
	m_text->setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x * half, bounds.position.y));
	m_text->setPosition(sf::Vector2f(context.windowWidth - marginX - recordHudOffsetLeft, marginY));
}
} // namespace game
