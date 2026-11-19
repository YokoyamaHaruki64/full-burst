#include "title.h"
#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "keyboard.h"
#include "main.h"
#include "vector3.h"
#include "utility.h"
#include "sound.h"
#include "fade.h"
#include "pause.h"

static constexpr Vector3 TEXT_POS{ WINDOW_WIDTH * 0.5f,750.0};
static constexpr Vector3 LOGO_POS_END{WINDOW_WIDTH * 0.5f,WINDOW_HEIGHT * 0.5 - 50.0f};
static constexpr Vector3 LOGO_POS_START{-WINDOW_WIDTH,LOGO_POS_END.y};
static constexpr int LOGO_ANIM_MAX{120};
static constexpr int TEXT_MAX_FRAME{80};

static int g_bgm;
static int g_enterSE;
static int g_bgTex;
static int g_logoTex;
static int g_textTex;
static int g_frame;

void TitleInitialize()
{
	PauseInitialize();
	g_frame = 0;
	//テクスチャ読み込み
	g_bgTex = TextureLoad(L"asset\\texture\\titleBG.jpg");
	g_logoTex = TextureLoad(L"asset\\texture\\logo.png");
	g_textTex = TextureLoad(L"asset\\texture\\pressSpace.png");
	g_bgm = LoadSound("asset\\sound\\titlBGM_Feather_of_the_Angel.wav");
	g_enterSE = LoadSound("asset\\sound\\toGame.wav");
	PlaySound(g_bgm,-1);
	SetVolume(g_bgm, 0.3f);
	SetVolume(g_enterSE, 0.5f);
}

void TitleFinalize()
{
	PauseFinalize();
	StopSound(g_bgm);
}

void TitleUpdate()
{
	if (GetFadeState() == FADE_NONE)
	{
		if(IsGamePaused())
		{
			PauseUpdate();
		}
		else
		{
			if (Keyboard_IsKeyTrigger(KK_SPACE))
			{
				SceneChange_Fade(SCENE_GAME, 60, 0);
				PlaySound(g_enterSE, 0);
			}
		}

		CheckPauseInput();
	}
	g_frame++;
}

void TitleDraw()
{

	SpriteDraw({ WINDOW_WIDTH / 2,WINDOW_HEIGHT / 2, }, { WINDOW_WIDTH,WINDOW_HEIGHT },
		Vector3::One(), 0.0f, { 1.0f,1.0f,1.0f,1.0f }, g_bgTex);

	float logo_t = Clamp(0.0f, 1.0f, (float)g_frame / LOGO_ANIM_MAX);
	logo_t = EaseOutBack(logo_t,3.0f);
	Vector3 logoPos = LOGO_POS_START;
	logoPos.x = LerpUnclamped(LOGO_POS_START.x, LOGO_POS_END.x, logo_t);

	Vector3 textPos = TEXT_POS;
	float alpha = 0.0f;
	if (!IsGamePaused())
	{
		float sin = sinf(g_frame * 0.03f);
		alpha = 0.5f + 0.5f * (1.0f - sin);
		textPos.y += sin * 5.0f;
	}
	SpriteDraw(logoPos, { 1536.0f,1024.0f }, { 0.8f,0.8f,1.0f }, 0.0f, { 1.0f,1.0f,1.0f,1.0f }, g_logoTex);
	SpriteDraw(textPos, { 3314.0f,848.0f }, { 0.25f,0.25f,1.0f }, 0.0f, { 1.0f,1.0f,1.0f,alpha }, g_textTex);

	if(IsGamePaused())
	{
		PauseDraw();
	}
}