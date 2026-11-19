#pragma once
#include <vector>
#include "directx.h"
#include "collision.h"

struct Block
{
	int type;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	BoxCollider collider;
};

static constexpr int FIELD_SIZE = 50;
static constexpr int HALL_SIZE = 26;
static constexpr Vector3 FIELD_CENTER = { 0.0f, 0.0f, FIELD_SIZE / 2.0f - 1.0f };

static constexpr int TABLE_SIZE = 6;
static constexpr float HALL_TO_TABLE_X = (HALL_SIZE / 2 + TABLE_SIZE);

static constexpr Vector3 FIELD_MIN = { -FIELD_SIZE / 2.0f, 0.0f, -1.0f };
static constexpr Vector3 FIELD_MAX = { FIELD_SIZE / 2.0f, 10.0f, FIELD_SIZE - 1.0f };

static constexpr Vector3 HALL_MIN = { FIELD_CENTER.x - HALL_SIZE / 2.0f, 0.0f, FIELD_CENTER.z - HALL_SIZE / 2.0f };
static constexpr Vector3 HALL_MAX = { FIELD_CENTER.x + HALL_SIZE / 2.0f, 0.0f, FIELD_CENTER.z + HALL_SIZE / 2.0f };

static constexpr Vector3 TABLE_OFFSET[4] = {
	{ -HALL_TO_TABLE_X, 1.0f,  TABLE_SIZE * 0.5f },
	{  HALL_TO_TABLE_X, 1.0f,  TABLE_SIZE * 0.5f },
	{ -HALL_TO_TABLE_X, 1.0f,  FIELD_SIZE - (TABLE_SIZE * 1.5f) },
	{  HALL_TO_TABLE_X, 1.0f,  FIELD_SIZE - (TABLE_SIZE * 1.5f) },
};


std::vector<Block>& GetBlocks();

void FieldInitialize();
void FieldFinalize();
void FieldUpdate();
void FieldShadowDraw();
void FieldDraw();

void CreateWall(Vector3 pos);

