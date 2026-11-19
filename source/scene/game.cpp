#include "directx.h"
#include "utility.h"
#include "title.h"
#include "sound.h"
#include "mouse.h"
#include "texture.h"
#include "fade.h"
#include "light.h"
#include "star.h"
#include "keyboard.h"
#include "camera.h"
#include "game.h"
#include "player.h"
#include "shader.h"
#include "field.h"
#include "effect.h"
#include "sprite.h"
#include "shooter.h"
#include "UI.h"
#include "boss.h"
#include "B_verticalTarget.h"
#include "B_verticalLaser.h"
#include "B_radialLaser.h"
#include "fieldDecal.h"
#include "starTrail.h"
#include "arrow.h"
#include "gameTimer.h"
#include "directFont.h"
#include "sky.h"
#include "pause.h"

static constexpr float BGM_VOLUME = 0.3f;
static const std::string TUTORIAL_TEXT =
"[WASD] 移動 \n"
"[Space] ジャンプ\n"
"[マウス] カメラ移動\n\n"
"発射台に乗ってWASD連打！";

static int g_bgm;

static int hitStopFrame = 0;
static int hitStopCounter = 0;
static bool g_captureSnapShot = false;

void GameInitialize()
{
	g_bgm = LoadSound("asset\\sound\\gameBGM_kamuinanashiki.wav");
	FieldInitialize();

	CameraInitilize();
	StarInitialize();
	PlayerInitialize();
	EffectInitialize();
	ShooterInitialize();
	UIInitialize();
	BossInitialize();
	VerticalTargetInitialize();
	VerticalLaserInitialize();
	RadialLaserInitialize();
	FieldDecalInitialize();
	StarTrailInitialize();
	ArrowInitialize();
	GameTimerInitialize();
	SkyInitialize();

	PauseInitialize();

	g_captureSnapShot = false;

}

void GameFinalize()
{
	StopSound(g_bgm);

	FieldFinalize();
	StarFinalize();
	PlayerFinalize();
	EffectFinalize();
	ShooterFinalize();
	BossFinalize();
	UIFinalize();
	VerticalTargetFinalize();
	VerticalLaserFinalize();
	RadialLaserFinalize();
	FieldDecalFinalize();
	StarTrailFinalize();
	ArrowFinalize();
	GameTimerFinalize();
	SkyFinalize();
	PauseFinalize();
}

void GameUpdate()
{

	if(hitStopFrame > 0)
	{
		hitStopCounter++;
		if(hitStopCounter >= hitStopFrame)
		{
			hitStopFrame = 0;
			hitStopCounter = 0;
		}
		return;
	}


	if (IsGamePaused())
	{
		PauseUpdate();
	}
	else
	{
		CameraUpdate();
		FieldUpdate();
		PlayerUpdate();
		EffectUpdate();
		ShooterUpdate();
		UIUpdate();
		BossUpdate();
		VerticalTargetUpdate();
		VerticalLaserUpdate();
		RadialLaserUpdate();
		FieldDecalUpdate();
		StarUpdate();
		StarTrailUpdate();
		ArrowUpdate();
		GameTimerUpdate();

		if (GetBossState() == BossState::NONE)
		{
			float toVolume = (1.0f - GetFadeAlpha()) * BGM_VOLUME;
			SetVolume(g_bgm, toVolume);
		}
	}

	CheckPauseInput();

}

void GameShadowDraw()
{
	LightInitialize();
	Vector3 lightDir{ 0.8f,-0.7f,0.6f };
	SetLightDirection(lightDir.Normalized().ToXMFLOAT3());

	ShadowBegin();

	PlayerShadowDraw();
	FieldShadowDraw();
	ShooterShadowDraw();
	ArrowShadowDraw();

	ShadowEnd();

}

void GameDraw()
{
	GameShadowDraw();

	EnableLight(false);
	SetLightColor({ 0.78f,0.73f,0.8f });

	Shader_SetLight(GetLight());
	StarDraw();
	SkyDraw();

	EnableLight(true);
	Shader_SetLight(GetLight());

	FieldDraw();
	PlayerDraw();
	ShooterDraw();
	ArrowDraw();
	BossDraw();
	FieldDecalDraw();
	VerticalTargetDraw();

	EnableLight(false);
	Shader_SetLight(GetLight());
	VerticalLaserDraw();
	RadialLaserDraw();

	EffectDraw();
	StarTrailDraw();

	if (g_captureSnapShot)
	{
		CreateResultSnapshot();
		g_captureSnapShot = false;
	}

	UIDraw();
	GameTimerDraw();

	if (GetBossState() == BossState::TUTORIAL)
	{
		WriteText(
			TUTORIAL_TEXT,
			{ 50.0f, WINDOW_HEIGHT - 250.0f },
			Color::WHITE(),
			36.0f,
			2,
			Color::BLACK()
		);
	}

	if(IsGamePaused())
	{
		PauseDraw();
	}

}

void HitStop(int frame)
{
	hitStopFrame = frame;
	hitStopCounter = 0;
}

bool IsHitStop()
{
	return hitStopFrame > 0;
}

void PlayGameBGM()
{
	PlaySound(g_bgm, -1);
	SetVolume(g_bgm, BGM_VOLUME);
}

void CaptureGameSnapshot()
{
	g_captureSnapShot = true;
}
