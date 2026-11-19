#include "gameTimer.h"
#include "directFont.h"
#include "utility.h"
#include "boss.h"

static float g_gameTime = 0.0f;

void GameTimerInitialize()
{
	g_gameTime = 0.0f;
}

void GameTimerFinalize()
{

}

void GameTimerUpdate()
{
	BossState state = GetBossState();
	if (state == BossState::DEFEAT ||
		state == BossState::NONE ||
		state == BossState::TUTORIAL)
	{
		return;
	}

	g_gameTime += Time::deltaTime;
}

void GameTimerDraw()
{
	int miniutes = static_cast<int>(g_gameTime) / 60;
	int seconds = static_cast<int>(g_gameTime) % 60;
	int milliseconds = static_cast<int>((g_gameTime - static_cast<int>(g_gameTime)) * 100);
	std::string timeText = std::format("Time: {:02}:{:02}.{:02}", miniutes, seconds, milliseconds);
	WriteText(timeText, { 50.0f, 50.0f }, Color(1.0f,1.0f,0.35f,1.0f),60.0f,
		3,Color::BLACK());
}

float GetGameTime()
{
	return g_gameTime;
}