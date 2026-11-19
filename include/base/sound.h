
#pragma once

#include <windows.h>
#include "xaudio2.h"
#define MAX_VOLUME 100


bool InitSound(HWND hWnd);
void UninitSound(void);

int LoadSound(const char* pFilename);
void PlaySound(int index, int loopCount);
void StopSound(int index);
void StopSoundAll(void);
void SetVolume(int index, float vol);

const int GetMasterVolume();
void SetMasterVolume(int vol);	// 0Å`100

