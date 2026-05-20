#include <optional>

#include <SFML/Graphics.hpp>

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800u, 600u }), "GAME");
	window.setFramerateLimit(60);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		window.clear(sf::Color(30, 30, 40));
		window.display();
	}

	return 0;
}
