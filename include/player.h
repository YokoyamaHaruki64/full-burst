#pragma once
#include "directx.h"
#include "collision.h"
#include "vector3.h"
#include "field.h"

enum class PlayerState
{
	NONE = -1,
	MOVE,
	CHARGE,
	SHOT,
	RETURN,
	KNOCKBACK,
	INVINCIBLE,
	DEFEAT,
};

static constexpr float MAX_SHOT_POWER = 30.0f;
static constexpr float MAX_PLAYER_HP = 20.0f;
static constexpr Vector3 RETURN_POSITIONS[4] =
{
	Vector3(0.0f,	1.0f, 5.0f),
	Vector3(19.0f,	1.0f, FIELD_CENTER.z),
	Vector3(0.0f,	1.0f, 43.0f),
	Vector3(-19.0f,	1.0f, FIELD_CENTER.z),
};

void PlayerInitialize();
void PlayerFinalize();
void PlayerUpdate();
void PlayerShadowDraw();
void PlayerDraw();

Vector3 GetPlayerPosision();
void SetPlayerPosision(const Vector3& pos);

PlayerState GetPlayerState();
void SetPlayerState(PlayerState state);

void SetShooter(float angle,int index);
int GetOnShooterIndex();
bool IsPlayerGround();

float GetShotPowerLength();
SphereCollider GetPlayerCollider();

void PlayerDamage(int damage, const Vector3& directionXZ, int knockBackFrame);
float GetPlayerCurrentHP();
