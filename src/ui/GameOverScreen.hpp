#pragma once

#include <optional>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include "app/GameContext.hpp"

namespace game
{
enum class GameOverAction
{
	None,
	Restart,
	Exit,
};

class GameOverScreen
{
public:
	explicit GameOverScreen(const GameContext& context);

	void Show(int record, int currentScore);
	GameOverAction ProcessEvent(sf::RenderWindow& window, const sf::Event& event);
	void Render(sf::RenderWindow& window, const GameContext& context) const;

private:
	bool LoadFont();
	void UpdateLayout(const GameContext& context);
	void CenterLabelOnButton(sf::Text& label, const sf::Shape& button) const;

	sf::RectangleShape m_overlay;
	sf::ConvexShape m_panel;
	sf::ConvexShape m_restartButton;
	sf::ConvexShape m_exitButton;

	sf::Font m_font;
	mutable std::optional<sf::Text> m_recordLabel;
	mutable std::optional<sf::Text> m_scoreLabel;
	mutable std::optional<sf::Text> m_restartLabel;
	mutable std::optional<sf::Text> m_exitLabel;
	bool m_fontReady = false;

	int m_record = hitPointsDepleted;
	int m_currentScore = hitPointsDepleted;
};
} // namespace game
