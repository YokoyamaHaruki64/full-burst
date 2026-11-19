#pragma once

#include "utility.h"
#include "collision.h"

struct RadialLaser
{
	Vector3 startPosition;	// 端点
	Vector3 centerPosition; // 中心点
	Vector3 rotation;		// 回転角
	Vector3 directionXZ;		// 上から見た方向

	float length;			// 長さ
	float pitch;			// 上下角度
	float startPitch;		// 開始時の上下角度
	float radius;			// 半径
	int startFrame;			// 発射したフレーム
	int maxFrame;			// 最大フレーム

	CapsuleCollider collider;

	bool isActive;
	int decalIndex = -1;
};

static constexpr int MAX_RADIAL_LASER = 2048;
static constexpr float RADIALLASER_RADIUS = 0.15f;
static constexpr int RADIAL_MAX_FRAME = 120;

void RadialLaserInitialize();
void RadialLaserFinalize();
void RadialLaserUpdate();
void RadialLaserDraw();
void SetRadialLaserDamage(float damage);


void CreateRadialLaser(const Vector3& startPos,const Vector3 dir, 
	const float radius = RADIALLASER_RADIUS,const int maxFrame = RADIAL_MAX_FRAME);
