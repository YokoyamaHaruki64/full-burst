#include "fade.h"
#include "texture.h"
#include "directx.h"
#include "sprite.h"
#include "shader.h"
#include "vector3.h"
#include "light.h"

struct Fade
{
	FADE_STATE state = FADE_NONE;
	FADE_STATE nextState = FADE_NONE;
	Vector3 pos;
	Vector3 size;
	float alpha = 0.0f;

	int fadeCnt = 0;		// カウンター
	int fadeTime = 0;		// fade完了にかかる時間
	int startDuration = 0;	// 始まるまでの時間
};

Fade g_fade;
Scene g_nextScene;

bool g_isChangeScene;
int g_fadeTex;


void FadeInitialize()
{
	g_fade.pos = Vector3{ WINDOW_WIDTH * 0.5f,WINDOW_HEIGHT * 0.5f };
	g_fade.size = Vector3{ WINDOW_WIDTH * 2.0f ,WINDOW_HEIGHT * 2.0f};
	g_isChangeScene = false;
	g_fade.alpha = 0.0f;
	g_fadeTex = TextureLoad(L"asset\\texture\\white.png");
}

void FadeFinalize()
{
}

void FadeUpdate()
{
	switch (g_fade.state)
	{
	case FADE_NONE:
		break;

	case FADE_WAIT:
		g_fade.fadeCnt++;
		if (g_fade.fadeCnt >= g_fade.startDuration)
		{
 			g_fade.fadeCnt = 0;
			g_fade.state = g_fade.nextState;
		}
		break;

	case FADE_IN:
		g_fade.fadeCnt++;
		g_fade.alpha = 1.0f - (float)g_fade.fadeCnt / g_fade.fadeTime;
		if (g_fade.fadeCnt >= g_fade.fadeTime)
		{
			g_fade.fadeCnt = 0;
			g_fade.state = FADE_NONE;
		}
		break;

	case FADE_OUT:
		g_fade.fadeCnt++;
		g_fade.alpha = (float)g_fade.fadeCnt / g_fade.fadeTime;
		if (g_fade.fadeCnt >= g_fade.fadeTime)
		{
			g_fade.fadeCnt = 0;
			if (g_isChangeScene)
			{
				g_fade.state = FADE_IN;
				SetScene(g_nextScene);
				g_isChangeScene = false;
			}
			else
			{
				g_fade.state = FADE_NONE;
			}
		}
		break;

	default:
		break;
	}
}

void FadeDraw()
{
	if (g_fade.state != FADE_NONE || g_fade.state != FADE_WAIT )
	{
		Light light;
		light.enable = false;
		Shader_SetLight(light);
		IsDepthEnable(true);
		SpriteDraw(g_fade.pos, g_fade.size,Vector3::One(), 0.0f, { 0.0f,0.0f,0.0f,g_fade.alpha }, g_fadeTex);
		IsDepthEnable(false);
	}
}

void SetFadeOut(int fadeTime,int daray)
{
	g_fade.fadeTime = fadeTime;
	g_fade.startDuration = daray;
	g_fade.state = FADE_OUT;
}

void SetFadeIn(int fadeTime,int daray)
{
	g_fade.fadeTime = fadeTime;
	g_fade.startDuration = daray;
	g_fade.state = FADE_WAIT;
	g_fade.nextState = FADE_IN;
}

void SceneChange_Fade(Scene nextScene,int fadeTime, int daray)
{
	g_fade.fadeTime = fadeTime;
	g_fade.startDuration = daray;
	g_fade.state = FADE_WAIT;
	g_fade.nextState = FADE_OUT;
	g_nextScene = nextScene;
	g_isChangeScene = true;
}

FADE_STATE GetFadeState()
{
	return g_fade.state;
}

float GetFadeAlpha()
{
	return g_fade.alpha;
}