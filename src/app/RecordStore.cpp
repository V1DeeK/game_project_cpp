#include "app/RecordStore.hpp"

#include <fstream>

#include "config/GameConstants.hpp"

namespace game
{
RecordStore::RecordStore()
{
	Load();
}

void RecordStore::Load()
{
	std::ifstream input(recordFileName);
	if (input >> m_highScore)
	{
		return;
	}

	m_highScore = hitPointsDepleted;
}

void RecordStore::Save() const
{
	std::ofstream output(recordFileName);
	output << m_highScore;
}

void RecordStore::ResetCurrentRun()
{
	m_elapsedSeconds = initialFireCooldown;
	m_currentScore = hitPointsDepleted;
	m_runActive = true;
}

void RecordStore::Update(float deltaTime)
{
	if (!m_runActive)
	{
		return;
	}

	m_elapsedSeconds += deltaTime;
	m_currentScore = static_cast<int>(m_elapsedSeconds);
}

void RecordStore::CommitCurrentRun()
{
	if (!m_runActive)
	{
		return;
	}

	if (m_currentScore > m_highScore)
	{
		m_highScore = m_currentScore;
		Save();
	}

	m_runActive = false;
}

int RecordStore::GetHighScore() const
{
	return m_highScore;
}

int RecordStore::GetCurrentScore() const
{
	return m_currentScore;
}
} // namespace game
