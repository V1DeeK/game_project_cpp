#include "entities/Bullet.hpp"

#include <algorithm>

#include "config/GameConstants.hpp"

namespace game
{
void Bullet::UpdateAll(std::vector<Bullet>& bullets, float deltaTime, const GameContext& context)
{
	for (auto& bullet : bullets)
	{
		bullet.Update(deltaTime);
	}

	RemoveOffScreen(bullets, context);
}

void Bullet::RemoveOffScreen(std::vector<Bullet>& bullets, const GameContext& context)
{
	bullets.erase(
		std::remove_if(
			bullets.begin(),
			bullets.end(),
			[&](const Bullet& bullet) { return bullet.IsOffScreen(context); }),
		bullets.end());
}

void Bullet::Update(float deltaTime)
{
	m_shape.move(m_velocity * deltaTime);
}

bool Bullet::IsOffScreen(const GameContext& context) const
{
	const float removeMargin = bulletOffScreenMargin * context.scale;
	const sf::Vector2f position = m_shape.getPosition();
	return position.x < -removeMargin
		|| position.x > context.windowWidth + removeMargin
		|| position.y < -removeMargin
		|| position.y > context.windowHeight + removeMargin;
}

void Bullet::Draw(sf::RenderWindow& window) const
{
	window.draw(m_shape);
}

sf::Vector2f Bullet::GetCenter() const
{
	const sf::FloatRect bounds = m_shape.getGlobalBounds();
	return sf::Vector2f(
		bounds.position.x + bounds.size.x * half,
		bounds.position.y + bounds.size.y * half);
}

sf::FloatRect Bullet::GetBounds() const
{
	return m_shape.getGlobalBounds();
}

bool Bullet::IntersectsConvexShape(const sf::ConvexShape& shape) const
{
	return shape.getGlobalBounds().contains(GetCenter());
}
} // namespace game
