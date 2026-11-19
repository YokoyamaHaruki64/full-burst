#pragma once
#include "collision.h"

enum class BossState
{
	PHASE1,
	PHASE2,
	PHASE3,
	PHASE4,

	PHASE_MAX,

	TUTORIAL,
	NONE,
	DEFEAT,
};

static constexpr float BOSS_MAX_HP = 100.0f;

void BossInitialize();
void BossFinalize();
void BossUpdate();
void BossDraw();

void DamageBoss(float damage);
SphereCollider GetBossCollider();
float GetBossCurrentHP();

BossState GetBossState();
