#pragma once

#include <SFML/Graphics/Color.hpp>

namespace game
{
inline constexpr sf::Color kColorPlayerShipFill(50, 120, 230);
inline constexpr sf::Color kColorPlayerShipOutline(120, 180, 255);
inline constexpr sf::Color kColorSatelliteFill(55, 95, 220);
inline constexpr sf::Color kColorSatelliteOutline(100, 150, 255);
inline constexpr sf::Color kColorAsteroidFill(139, 90, 43);
inline constexpr sf::Color kColorAsteroidOutline(100, 65, 30);
inline constexpr sf::Color kColorNeutralFill(35, 130, 55);
inline constexpr sf::Color kColorNeutralOutline(20, 90, 40);
inline constexpr sf::Color kColorEnemyFill(200, 45, 45);
inline constexpr sf::Color kColorEnemyOutline(255, 90, 90);
inline constexpr sf::Color kColorBullet(255, 220, 50);
inline constexpr sf::Color kColorHpBackground(90, 20, 20);
inline constexpr sf::Color kColorHpForeground(0, 200, 0);
inline constexpr sf::Color kColorHpDamaged(200, 40, 40);
} // namespace game
