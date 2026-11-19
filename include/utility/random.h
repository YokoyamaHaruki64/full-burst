#pragma once
#include "vector3.h"

// ===========================================
// InitRandom（乱数初期化）
// ===========================================
void InitRandom();

// ===========================================
// RandomRange（乱数）
// -------------------------------------------
// min ～ max の範囲でランダムなfloatを返す（maxは含まない）
// ===========================================
float RandomRange(float min, float max);

// ===========================================
// RandomRange（乱数）
// -------------------------------------------
// min ～ max の範囲でランダムなintを返す（maxを含む）
// ===========================================
int RandomRange(int min, int max);

Vector3 RandomVector3(const Vector3& min, const Vector3& max);



