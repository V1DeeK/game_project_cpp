#include "entities/NeutralShip.hpp"

#include <algorithm>
#include <cmath>
#include <random>

#include "collision/Geometry.hpp"
#include "config/Colors.hpp"
#include "config/GameConstants.hpp"

namespace
{
sf::Vector2f GetNeutralShipSize(const game::GameContext& context)
{
	return sf::Vector2f(
		context.shipWidth * game::neutralShipSizeMultiplier,
		context.shipHeight * game::neutralShipSizeMultiplier);
}

sf::Vector2f CreateOffScreenSpawnPosition(
	const game::GameContext& context,
	float spawnMargin,
	std::mt19937& rng)
{
	std::uniform_real_distribution<float> positionX(game::screenOrigin, context.windowWidth);
	std::uniform_real_distribution<float> positionY(game::screenOrigin, context.windowHeight);
	std::uniform_int_distribution<int> edgeDist(game::screenEdgeTop, game::screenEdgeLeft);

	switch (edgeDist(rng))
	{
	case game::screenEdgeTop:
		return sf::Vector2f(positionX(rng), -spawnMargin);
	case game::screenEdgeRight:
		return sf::Vector2f(context.windowWidth + spawnMargin, positionY(rng));
	case game::screenEdgeBottom:
		return sf::Vector2f(positionX(rng), context.windowHeight + spawnMargin);
	default:
		return sf::Vector2f(-spawnMargin, positionY(rng));
	}
}

sf::ConvexShape CreateNeutralTriangleShape(const game::GameContext& context)
{
	const sf::Vector2f size = GetNeutralShipSize(context);
	const float width = size.x;
	const float height = size.y;

	sf::ConvexShape shape(game::shipTriangleVertexCount);
	shape.setPoint(
		game::triangleVertexNose,
		sf::Vector2f(width * game::half, game::screenOrigin));
	shape.setPoint(game::triangleVertexBottomLeft, sf::Vector2f(game::screenOrigin, height));
	shape.setPoint(game::triangleVertexBottomRight, sf::Vector2f(width, height));
	shape.setOrigin(sf::Vector2f(width * game::half, height * game::half));
	shape.setFillColor(game::kColorNeutralFill);
	shape.setOutlineColor(game::kColorNeutralOutline);
	shape.setOutlineThickness(game::defaultOutlineThickness);
	return shape;
}

sf::Vector2f CreateInboundVelocity(sf::Vector2f spawnPosition, const game::GameContext& context, float speed, std::mt19937& rng)
{
	const sf::Vector2f screenCenter(context.windowWidth * game::half, context.windowHeight * game::half);
	sf::Vector2f direction = screenCenter - spawnPosition;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > game::directionEpsilon)
	{
		direction /= length;
	}

	std::uniform_real_distribution<float> angleOffset(
		game::neutralSpawnAngleOffsetMin,
		game::neutralSpawnAngleOffsetMax);
	const float offsetAngle = angleOffset(rng);
	const float cosA = std::cos(offsetAngle);
	const float sinA = std::sin(offsetAngle);
	const sf::Vector2f rotated(
		direction.x * cosA - direction.y * sinA,
		direction.x * sinA + direction.y * cosA);

	return rotated * speed;
}
} // namespace

namespace game
{
std::vector<NeutralShip> NeutralShip::CreateInitialFleet(const GameContext& context, std::mt19937& rng)
{
	std::vector<NeutralShip> neutrals;
	neutrals.reserve(neutralShipCount);
	for (int i = hitPointsDepleted; i < neutralShipCount; ++i)
	{
		neutrals.push_back(CreateRandom(context, rng));
	}
	return neutrals;
}

void NeutralShip::UpdateAll(
	std::vector<NeutralShip>& neutrals,
	float deltaTime,
	const GameContext& context,
	std::mt19937& rng)
{
	for (auto& neutral : neutrals)
	{
		neutral.m_shape.move(neutral.m_velocity * deltaTime);
	}

	const float removeMargin = neutralOffScreenRemoveMargin * context.scale;
	neutrals.erase(
		std::remove_if(
			neutrals.begin(),
			neutrals.end(),
			[&](const NeutralShip& neutral) {
				return IsFullyOutsideScreen(neutral.GetBounds(), context, removeMargin);
			}),
		neutrals.end());

	while (static_cast<int>(neutrals.size()) < neutralShipCount)
	{
		neutrals.push_back(CreateRandom(context, rng));
	}
}

NeutralShip NeutralShip::CreateRandom(const GameContext& context, std::mt19937& rng)
{
	const sf::Vector2f shipSize = GetNeutralShipSize(context);
	const float spawnMargin = std::max(shipSize.x, shipSize.y) + offScreenSpawnMarginExtra * context.scale;

	const sf::Vector2f position = CreateOffScreenSpawnPosition(context, spawnMargin, rng);
	const float speed = neutralMoveSpeed * context.scale;
	const sf::Vector2f velocity = CreateInboundVelocity(position, context, speed, rng);

	sf::ConvexShape shape = CreateNeutralTriangleShape(context);
	shape.setPosition(position);

	const float angleDegrees = std::atan2(velocity.y, velocity.x) * degreesPerRadian + shipRotationOffsetDegrees;
	shape.setRotation(sf::degrees(angleDegrees));

	NeutralShip neutral;
	neutral.m_shape = std::move(shape);
	neutral.m_velocity = velocity;
	return neutral;
}

void NeutralShip::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

bool NeutralShip::IsOnScreen(const GameContext& context) const
{
	return IsShapeOnScreen(m_shape, context);
}

sf::FloatRect NeutralShip::GetBounds() const
{
	return m_shape.getGlobalBounds();
}

const sf::ConvexShape& NeutralShip::GetShape() const
{
	return m_shape;
}

bool NeutralShip::IntersectsRect(const sf::FloatRect& rect) const
{
	return RectsIntersect(GetBounds(), rect);
}
} // namespace game
