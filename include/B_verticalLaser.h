#pragma once
#include "utility.h"
#include "collision.h"

struct VerticalLaser
{
	Vector3 position;
	SylinderCollider collider;

	bool isActive;
	float radius;
};

static constexpr int MAX_VERTICAL_LASER = 2048;

void VerticalLaserInitialize();
void VerticalLaserFinalize();
void VerticalLaserUpdate();
void VerticalLaserDraw();

void CreateVerticalLaser(const Vector3& pos, const float radius);
VerticalLaser* GetVerticalLasers();

