#pragma once
#include "directx.h"
#include "vector3.h"

enum class EffectType
{
	EXPLOSION,
	EXPLOSION_BIG,

	MAX
};

void EffectInitialize();
void EffectFinalize();
void EffectUpdate();
void EffectDraw();

void CreateEffect(const Vector3& pos, const float scale, EffectType type);

