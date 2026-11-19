#include "UI.h"
#include "utility.h"
#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "player.h"
#include "camera.h"
#include "boss.h"

static constexpr float POWER_GAUGE_LENGTH = 1.5f;	// 3D空間上のゲージの長さ
static constexpr float PLAYER_HP_GAUGE_LENGTH = 300.0f;	// ピクセル単位
static constexpr float PLAYER_HP_GAUGE_HEIGHT = 30.0f;	// ピクセル単位

static constexpr float BOSS_HP_GAUGE_LENGTH = 600.0f;	// ピクセル単位
static constexpr float BOSS_HP_GAUGE_HEIGHT = 30.0f;	// ピクセル単位

static int g_whiteTex;
static int g_keyTex[4];
static int g_keyPressTex;

static FrameTime* g_keyAnim;
static bool g_keyPress = false;

void UIInitialize()
{
	g_whiteTex = TextureLoad(L"asset\\texture\\white.png");
	g_keyTex[0] = TextureLoad(L"asset\\texture\\key_w.png");
	g_keyTex[1] = TextureLoad(L"asset\\texture\\key_a.png");
	g_keyTex[2] = TextureLoad(L"asset\\texture\\key_s.png");
	g_keyTex[3] = TextureLoad(L"asset\\texture\\key_d.png");
	g_keyPressTex = TextureLoad(L"asset\\texture\\keyPress.png");

	g_keyAnim = new FrameTime(15);
}

void UIFinalize()
{
	delete g_keyAnim;
}

void UIUpdate()
{
	if(g_keyAnim->IsFinished())
	{
		g_keyPress = !g_keyPress;
		g_keyAnim->Reset();
	}
}

void ChargeUIDraw()
{
	// ショットパワーゲージの描画
	Vector3 pos = GetPlayerPosision();
	Vector3 scale = Vector3(POWER_GAUGE_LENGTH, 0.2f, 0.2f);
	pos.y += 0.5f;

	// アウトラインの描画
	BillboardDraw(pos, scale + Vector3(0.06f, 0.05f, 0.0f), 0.0f, Color(0.6f, 0.6f, 0.7f, 1.0f), g_whiteTex);

	// 下地の描画
	BillboardDraw(pos,scale,0.0f,Color::BLACK(),g_whiteTex);

	// ゲージの描画
	float length = GetShotPowerLength();
	float t = length / MAX_SHOT_POWER;
	float correctedT = t * POWER_GAUGE_LENGTH;

	scale.x *= t;
	Vector3 offset;
	offset.x = -((POWER_GAUGE_LENGTH - correctedT) * 0.5f);
	BillboardDraw(pos, scale, 0.0f, Color::RED(), g_whiteTex, offset);

	// キー表示
	Vector3 keyPos = Vector3(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f - 200.0f, 0.0f);
	Color keyColor = Color::WHITE();
	scale = Vector3::One();

	// 押すアニメーション
	if (g_keyPress)
	{
		keyColor = Color(0.7f, 0.7f, 0.7f, 1.0f);
		scale = Vector3(0.7f, 0.7f, 1.0f);

		SpriteDraw(keyPos, Vector3(75.0f, 75.0f, 1.0f),
			Vector3::One(), 0.0f, Color::WHITE(), g_keyPressTex);
	}

	SpriteDraw(keyPos, Vector3(50.0f, 50.0f, 1.0f), scale,
		0.0f, keyColor, g_keyTex[0]);

	keyPos.y += 60.0f;
	keyPos.x -= 60.0f;
	for (int i = 0; i < 3; i++)
	{
		if (g_keyPress)
		{
			SpriteDraw(keyPos, Vector3(75.0f, 75.0f, 1.0f),
				Vector3::One(), 0.0f, Color::WHITE(), g_keyPressTex);
		}

		SpriteDraw(keyPos, Vector3(50.0f, 50.0f, 1.0f), scale,
			0.0f, keyColor, g_keyTex[i + 1]);

		keyPos.x += 60.0f;
	}

}

void PlayerHPDraw()
{
	float t = GetPlayerCurrentHP() / MAX_PLAYER_HP;
	float correctedLength = t * PLAYER_HP_GAUGE_LENGTH;

	// アウトラインの描画
	Vector3 pos = Vector3(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 80.0f, 0.0f);
	Vector3 size = Vector3(PLAYER_HP_GAUGE_LENGTH, PLAYER_HP_GAUGE_HEIGHT, 1.0f);
	SpriteDraw(pos, size + Vector3(4.0f,4.0f), Vector3::One(), 0.0f, Color(0.2f, 0.2f, 0.3f, 1.0f), g_whiteTex);
	// 下地の描画
	SpriteDraw(pos, size, Vector3::One(), 0.0f, Color::BLACK(), g_whiteTex);
	
	// HPゲージの描画
	size.x *= t;
	pos.x -= (PLAYER_HP_GAUGE_LENGTH - correctedLength) * 0.5f;
	SpriteDraw(pos, size, Vector3::One(), 0.0f,Color(0.5f,1.0f,1.0f), g_whiteTex);

}

void BossHPDraw()
{
	float t = GetBossCurrentHP() / BOSS_MAX_HP;
	float correctedLength = t * BOSS_HP_GAUGE_LENGTH;
	// アウトラインの描画
	Vector3 pos = Vector3(WINDOW_WIDTH * 0.8f, 60.0f, 0.0f);
	Vector3 size = Vector3(BOSS_HP_GAUGE_LENGTH, BOSS_HP_GAUGE_HEIGHT, 1.0f);
	SpriteDraw(pos, size + Vector3(6.0f, 6.0f), Vector3::One(), 0.0f,Color(0.3f, 0.2f, 0.2f,1.0f), g_whiteTex);

	// 下地の描画
	SpriteDraw(pos, size, Vector3::One(), 0.0f, Color::BLACK(), g_whiteTex);

	// HPゲージの描画
	size.x *= t;
	pos.x -= (BOSS_HP_GAUGE_LENGTH - correctedLength) * 0.5f;
	SpriteDraw(pos, size, Vector3::One(), 0.0f, Color::RED(), g_whiteTex);
}

void UIDraw()
{
	IsDepthEnable(false);
	Rimlight rim;
	Shader_SetRimlight(rim);

	if (GetPlayerState() == PlayerState::CHARGE)
	{
		ChargeUIDraw();
	}
	BossHPDraw();
	PlayerHPDraw();
}

