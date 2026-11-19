#include "result.h"
#include "directx.h"
#include "sound.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "keyboard.h"
#include "main.h"
#include "vector3.h"
#include "utility.h"
#include "fade.h"
#include "directFont.h"


static constexpr Vector3 TEXT_POS{ 300.0f,750.0f };
static constexpr Vector3 TIME_POS{ 400.0f, 500.0f };

static int g_bgm;
static int g_failTex;
static int g_successTex;
static int g_enterSE;
static int g_textTex;
static int g_frame;
static bool g_isSuccess = false;
static float g_clearTime;
static float g_displayTime;
static float g_addTime = 0.0f;


void ResultInitialize()
{
	g_frame = 0;
	g_textTex = TextureLoad(L"asset\\texture\\pressSpace.png");
	if(g_isSuccess)
	{
		// 成功時の処理
		g_bgm = LoadSound("asset\\sound\\success_quantum.wav");
		g_successTex = TextureLoad(L"asset\\texture\\success.png");
		SetVolume(g_bgm, 0.1f);
	}
	else
	{
		// 失敗時の処理
		g_failTex = TextureLoad(L"asset\\texture\\failed.png");
		g_bgm = LoadSound("asset\\sound\\failed_musukarinohana.wav");	
		SetVolume(g_bgm, 0.3f);

	}
	g_enterSE = LoadSound("asset\\sound\\toGame.wav");
	PlaySound(g_bgm, -1);

	g_displayTime = 0.0f;
	g_addTime = 0.0f;
}

void ResultFinalize()
{
	StopSound(g_bgm);
	g_clearTime = 0.0f;
	g_isSuccess = false;
}

void ResultUpdate()
{
	if (g_isSuccess)
	{
		// クリアタイムの表示更新
		float diff = g_clearTime - g_displayTime;
		g_addTime = std::max(diff / 120, g_addTime);	// 最大2秒でクリアタイムに到達するように調整
		g_displayTime += g_addTime;
	}

	if(g_displayTime > g_clearTime)
	{
		g_displayTime = g_clearTime;
	}

	if (GetFadeState() == FADE_NONE)
	{
		if (Keyboard_IsKeyTrigger(KK_SPACE))
		{
			SceneChange_Fade(SCENE_TITLE, 60, 0);
			PlaySound(g_enterSE,0);
		}
	}
	g_frame++;
}

void ResultDraw()
{
	IsDepthEnable(false);
	SetBlendState(NOT_ALPHA);
	SpriteDraw(
		{ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f },
		{ WINDOW_WIDTH,WINDOW_HEIGHT },
		Vector3::One(),
		0.0f,
		Color::WHITE().SubRGB(0.5f),// 暗くする
		GetResultSnapshotSRV()
	);
	SetBlendState(ALPHABLEND);

	Vector3 textPos = TEXT_POS;
	float sin = sinf(g_frame * 0.03f);
	float alpha = 0.5f + 0.5f * (1.0f - sin);
	textPos.y += sin * 5.0f;
	SpriteDraw(textPos, { 3314.0f,848.0f }, { 0.1f,0.1f,1.0f }, 0.0f, { 1.0f,1.0f,1.0f,alpha }, g_textTex);

	// 成功/失敗表示
	int tex = g_isSuccess ? g_successTex : g_failTex;
	Vector2 size = g_isSuccess ? Vector2{ 2412.0f,848.0f } : Vector2{ 2475.0f,848.0f };
	SpriteDraw({ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.5f }, size,
		{ 0.5f,0.5f,1.0f }, 0.0f, Color::WHITE(), tex);

	// クリア時のみ表示
	if (g_isSuccess)
	{
		// クリアタイム表示
		int miniutes = static_cast<int>(g_displayTime) / 60;
		int seconds = static_cast<int>(g_displayTime) % 60;
		int milliseconds = static_cast<int>((g_displayTime - static_cast<int>(g_displayTime)) * 100);
		std::string timeText = std::format("Clear Time: {:02}:{:02}.{:02}", miniutes, seconds, milliseconds);

		WriteText(timeText, TIME_POS, Color(1.0f, 1.0f, 0.35f, 1.0f), 80.0f,
			3, Color::BLACK());
	}

}

void SetBattleSuccess(bool isSuccess)
{
	g_isSuccess = isSuccess;
}

void SetResultClearTime(float clearTime)
{
	g_clearTime = clearTime;
}