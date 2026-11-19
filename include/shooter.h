#pragma once
#include "vector3.h"
#include "field.h"


static constexpr float SHOOTER_OFFSET = TABLE_SIZE / 2.0f - 1.0f;

static constexpr Vector3 SHOOTER_POSITION[4] = {
	TABLE_OFFSET[0] + Vector3(0.0f, 0.2f, SHOOTER_OFFSET),
	TABLE_OFFSET[1] + Vector3(0.0f, 0.2f, SHOOTER_OFFSET),
	TABLE_OFFSET[3] + Vector3(0.0f, 0.2f, SHOOTER_OFFSET),
	TABLE_OFFSET[2] + Vector3(0.0f, 0.2f, SHOOTER_OFFSET),
};
void ShooterInitialize();
void ShooterFinalize();
void ShooterUpdate();
void ShooterShadowDraw();
void ShooterDraw();

void SetAllShooterActive(bool active);
void SetShooterActive(int index, bool active);
bool IsShooterActive(int index);
