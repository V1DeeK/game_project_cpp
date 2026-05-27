#pragma once

#include "config/GameConstants.hpp"

namespace game
{
class RecordStore
{
public:
	RecordStore();

	void Load();
	void Save() const;

	void ResetCurrentRun();
	void Update(float deltaTime);
	void CommitCurrentRun();

	int GetHighScore() const;
	int GetCurrentScore() const;

private:
	int m_highScore = hitPointsDepleted;
	float m_elapsedSeconds = initialFireCooldown;
	int m_currentScore = hitPointsDepleted;
	bool m_runActive = false;
};
} // namespace game
