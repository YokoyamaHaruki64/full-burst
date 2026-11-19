#pragma once
#include "main.h"

enum FADE_STATE
{
	FADE_NONE,
	FADE_WAIT,
	FADE_IN,
	FADE_OUT,

};



void FadeInitialize();
void FadeFinalize();
void FadeUpdate();
void FadeDraw();

FADE_STATE GetFadeState();
void SetFadeOut(int fadeTime, int daray);
void SetFadeIn(int fadeTime, int daray);
void SceneChange_Fade(Scene nextScene, int fadeTime, int daray);
float GetFadeAlpha();