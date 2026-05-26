#pragma once

#include <random>
#include <vector>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
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
	void UpdateLayout(const GameContext& context);

	Earth m_earth;
	sf::Vector2f m_earthCenter;
	std::vector<Satellite> m_satellites;
	std::vector<Asteroid> m_asteroids;

	sf::RectangleShape m_startButton;
	sf::RectangleShape m_exitButton;
	sf::ConvexShape m_startIcon;
};
} // namespace game

