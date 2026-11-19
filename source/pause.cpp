#include "pause.h"
#include "input.h"
#include "texture.h"
#include "UISystem.h"
#include "directx.h"
#include "main.h"
#include "sound.h"
#include "fade.h"
#include "sprite.h"
#include "directFont.h"	
#include "camera.h"
enum PauseUI
{
	RESUME_BUTTON,
	QUIT_BUTTON,	// ゲーム中はタイトルへ、タイトルではゲーム終了
	VOLUME_SLIDER,
	SENSI_SLIDER,
	PAUSE_UI_MAX,
};

static constexpr Vector2 RESUME_BUTTON_POSITION{ WINDOW_WIDTH - 300.0f, WINDOW_HEIGHT  - 100.0f };
static constexpr Vector2 QUIT_BUTTON_POSITION{ 300.0f, WINDOW_HEIGHT - 100.0f };
static constexpr Vector2 VOL_SLIDER_POSITION{ 500.0f, WINDOW_HEIGHT / 2.0f };
static constexpr Vector2 SENSI_SLIDER_POSITION{ 500.0f, WINDOW_HEIGHT / 2.0f - 75.0f};
static constexpr float UI_SCALE = 0.15f;
static constexpr float SENSITIVITY_SCALE = 100.0f;

void SwitchPause();

static bool g_isPaused = false;
static bool g_prevIsPaused = false;
static int g_UIIDs[PAUSE_UI_MAX];
static Scene g_currentScene = SCENE_NONE;
static int g_whiteTex;

void PauseInitialize()
{
	g_isPaused = false;
	g_prevIsPaused = false;
	g_currentScene = GetScene();
	g_whiteTex = TextureLoad(L"asset\\texture\\white.png");

	// UIの初期化
	UISystemInitialize();

	g_UIIDs[RESUME_BUTTON] = CreateButton(
		RESUME_BUTTON_POSITION,
		{ UI_SCALE,UI_SCALE }, 0.0f, TextureLoad(L"asset\\texture\\resumeButton.png"));

	int quitButtonTexture = -1;
	if(g_currentScene == SCENE_GAME)
	{
		quitButtonTexture = TextureLoad(L"asset\\texture\\toTitleButton.png");
	}
	else if(g_currentScene == SCENE_TITLE)
	{
		quitButtonTexture = TextureLoad(L"asset\\texture\\quitButton.png");
	}

	g_UIIDs[QUIT_BUTTON] = CreateButton(
		QUIT_BUTTON_POSITION,
		{ UI_SCALE,UI_SCALE }, 0.0f, quitButtonTexture);

	g_UIIDs[VOLUME_SLIDER] = CreateSlider(
		VOL_SLIDER_POSITION,
		{ UI_SCALE,UI_SCALE }, 0.0f,
		TextureLoad(L"asset\\texture\\sliderHandle.png"),
		TextureLoad(L"asset\\texture\\sliderBar.png"),
		0, MAX_VOLUME, GetMasterVolume());

	g_UIIDs[SENSI_SLIDER] = CreateSlider(
		SENSI_SLIDER_POSITION,
		{ UI_SCALE,UI_SCALE }, 0.0f,
		TextureLoad(L"asset\\texture\\sliderHandle.png"),
		TextureLoad(L"asset\\texture\\sliderBar.png"),
		MIN_SENSITIVITY, MAX_SENSITIVITY, GetSensitivity());
}

void PauseFinalize()
{
	UISystemFinalize();
	g_isPaused = false;
	g_prevIsPaused = false;
}

void PauseUpdate()
{
	UISystemUpdate();

	ButtonUIInfo resumeButtonInfo = GetButtonInfo(g_UIIDs[RESUME_BUTTON]);
	ButtonUIInfo quitButtonInfo = GetButtonInfo(g_UIIDs[QUIT_BUTTON]);
	SliderUIInfo volumeSliderInfo = GetSliderInfo(g_UIIDs[VOLUME_SLIDER]);
	SliderUIInfo sensiSliderInfo = GetSliderInfo(g_UIIDs[SENSI_SLIDER]);
	if (resumeButtonInfo.isPressAndRelease)
	{
		SwitchPause();
	}

	if (quitButtonInfo.isPressAndRelease)
	{
		if(g_currentScene == SCENE_GAME)
		{
			SwitchPause();
			SceneChange_Fade(SCENE_TITLE, 60, 0);
		}
		else if(g_currentScene == SCENE_TITLE)
		{
			EndApplication();
		}
	}

	if (volumeSliderInfo.isDragging)
	{
		SetMasterVolume(volumeSliderInfo.value);
	}

	if(sensiSliderInfo.isDragging)
	{
		SetSensitivity(sensiSliderInfo.value);
	}
}

void PauseDraw()
{
	SpriteDraw(
		{ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f },
		{ WINDOW_WIDTH, WINDOW_HEIGHT },
		{ 1.0f,1.0f },
		0.0f,
		Color(0.0f, 0.0f, 0.0f, 0.5f),
		g_whiteTex);

	UISystemDraw();

	Vector2 pos = { VOL_SLIDER_POSITION.x, VOL_SLIDER_POSITION.y};
	pos.x -= GetScaledSliderSize(g_UIIDs[VOLUME_SLIDER]).x * 0.5f + 125.0f;
	pos.y -= 18.0f;
	WriteText("音量",pos, Color::WHITE(), 36.0f, 2, Color::BLACK());
	pos.x = VOL_SLIDER_POSITION.x + GetScaledSliderSize(g_UIIDs[VOLUME_SLIDER]).x * 0.5f + 25.0f;
	WriteText(std::to_string((int)GetMasterVolume()).c_str(), pos, Color::WHITE(), 36.0f, 2, Color::BLACK());

	pos = { SENSI_SLIDER_POSITION.x, SENSI_SLIDER_POSITION.y };
	pos.x -= GetScaledSliderSize(g_UIIDs[SENSI_SLIDER]).x * 0.5f + 200.0f;
	pos.y -= 18.0f;
	WriteText("マウス感度", pos, Color::WHITE(), 36.0f, 2, Color::BLACK());
	pos.x = SENSI_SLIDER_POSITION.x + GetScaledSliderSize(g_UIIDs[SENSI_SLIDER]).x * 0.5f + 25.0f;
	float showSensitivity = GetSensitivity() * SENSITIVITY_SCALE;
	WriteText(std::format("{:.2f}", showSensitivity).c_str(), pos, Color::WHITE(), 36.0f, 2, Color::BLACK());
}

void CheckPauseInput()
{
	if (Input::Key_Trigger(KK_ESCAPE) || Input::Key_Trigger(KK_P))
	{
		SwitchPause();
	}
}

void SwitchPause()
{
	g_isPaused = !g_isPaused;
	if (g_isPaused)
	{
		SetCursorCenter();
	}
	else
	{
		SetCursorPos(0,0);
	}
}

bool IsGamePaused()
{
	return g_isPaused;
}
