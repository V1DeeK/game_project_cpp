#pragma once

#include <optional>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include "app/GameContext.hpp"

namespace game
{
class RecordDisplay
{
public:
	RecordDisplay();

	void RenderRecord(sf::RenderWindow& window, const GameContext& context, int highScore) const;
	void RenderScore(sf::RenderWindow& window, const GameContext& context, int currentScore) const;

private:
	bool LoadFont();
	void UpdateLayout(const GameContext& context, const char* label, int value) const;

	sf::Font m_font;
	mutable std::optional<sf::Text> m_text;
	bool m_ready = false;
};
} // namespace game
