#pragma once

#include <optional>
#include <random>
#include <vector>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include "app/GameContext.hpp"
#include "entities/Asteroid.hpp"
#include "entities/Earth.hpp"
#include "entities/Satellite.hpp"

namespace game
{
enum class StartScreenAction
{
	None,
	StartGame,
	Exit,
};

class StartScreen
{
public:
	explicit StartScreen(const GameContext& context, std::mt19937& rng);

	StartScreenAction ProcessEvent(sf::RenderWindow& window, const sf::Event& event);
	void Update(float deltaTime, const GameContext& context, std::mt19937& rng);
	void Render(sf::RenderWindow& window, const GameContext& context) const;

private:
	bool LoadFont();
	void UpdateLayout(const GameContext& context);
	void CenterLabelOnButton(sf::Text& label, const sf::Shape& button) const;

	Earth m_earth;
	sf::Vector2f m_earthCenter;
	std::vector<Satellite> m_satellites;
	std::vector<Asteroid> m_asteroids;

	sf::ConvexShape m_startButton;
	sf::ConvexShape m_exitButton;

	sf::Font m_font;
	std::optional<sf::Text> m_startLabel;
	std::optional<sf::Text> m_exitLabel;
	bool m_fontReady = false;
};
} // namespace game
