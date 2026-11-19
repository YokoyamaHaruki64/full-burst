#pragma once

#include "utility.h"
constexpr float EASE_BACK_OVERSHOOT = 1.70158f;


// ============= Quadratic（二次関数）=============
// 動きが比較的単純
// ================================================

// ゆっくりスタートして加速 段階1
inline float EaseInQuad(float t) 
{
	Clamp01(t);
	return t * t;
}

// 速く始まり、ゆっくり止まる 段階1
inline float EaseOutQuad(float t) 
{
	Clamp01(t);
	return 1.0f - (1.0f - t) * (1.0f - t); 
}

// 加速→減速 段階1
inline float EaseInOutQuad(float t) 
{ 
	Clamp01(t);
	return t < 0.5 ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; 
}

// ============= Cubic（三次関数）=============
// Quadよりも緩急が強い動き。より滑らか。
// ===========================================

// ゆっくりスタートして加速 段階2
inline float EaseInCubic(float t) 
{ 
	Clamp01(t);
	return t * t * t;
}

// 速く始まり、ゆっくり止まる 段階2
inline float EaseOutCubic(float t) 
{
	Clamp01(t);
	float u = t - 1.0f;
	return u * u * u + 1.0f;
}

// 加速→減速 段階2
inline float EaseInOutCubic(float t) 
{
	Clamp01(t);
	float u = 2.0f * t - 2.0f;
	return t < 0.5f ? 4.0f * t * t * t : 0.5f * u * u * u + 1.0f;
}

// ============= Quart（四次関数）=============
// より滑らかで急なカーブ。自然な動きに近い。
// ===========================================

// ゆっくりスタートして加速 段階3
inline float EaseInQuart(float t)
{ 
	Clamp01(t);
	return t * t * t * t; 
}

// 速く始まり、ゆっくり止まる 段階3
inline float EaseOutQuart(float t) 
{
	Clamp01(t);
	float u = t - 1.0f;
	return 1.0f - u * u * u * u;
}

// 加速→減速 段階3
inline float EaseInOutQuart(float t) 
{
	Clamp01(t);
	float u = t - 1.0f;
	return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * u * u * u * u;
}

// ============= Quint（五次関数）=============
// より強い加速・減速。滑らかで印象的な動き。
// ===========================================

// ゆっくりスタートして加速 段階4
inline float EaseInQuint(float t) 
{ 
	Clamp01(t);
	return t * t * t * t * t;
}

// 速く始まり、ゆっくり止まる 段階4
inline float EaseOutQuint(float t) 
{
	Clamp01(t);
	float u = t - 1.0f;
	return 1 + u * u * u * u * u;
}

// 加速→減速 段階4
inline float EaseInOutQuint(float t) 
{
	Clamp01(t);
	float u = t - 1.0f;
	return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f + 16.0f * u * u * u * u * u;
}

// ============= Expo（指数関数）=============
// 急激な加速・減速。スタートやエンドが一気に動く
// ==========================================

// ゆっくりスタートして加速 段階5
inline float EaseInExpo(float t)
{
	Clamp01(t);
	return pow(2.0f, 10.0f * (t - 1.0f));
}

// 速く始まり、ゆっくり止まる 段階5
inline float EaseOutExpo(float t)
{
	Clamp01(t);
	return 1.0f - pow(2.0f, -10.0f * t);
}

// 加速→減速 段階5
inline float EaseInOutExpo(float t)
{
	Clamp01(t);
	return (t < 0.5f)
		? 0.5f * pow(2.0f, (20.0f * t) - 10.0f)
		: 1.0f - 0.5f * pow(2.0f, (-20.0f * t) + 10.0f);
}

// ============= Circ（円形）=============
// 円弧を描くような動き。滑らかで柔らかい。
// =======================================

// ゆっくりスタートして加速 円形の前半
inline float EaseInCirc(float t)
{
	Clamp01(t);
	return 1.0f - sqrtf(1.0f - (t * t));
}

// 速く始まり、ゆっくり止まる 円形の後半
inline float EaseOutCirc(float t)
{
	Clamp01(t);
	float u = t - 1.0f;
	return sqrtf(1.0f - (u * u));
}

// 加速→減速 円形を沿う感じ
inline float EaseInOutCirc(float t)
{
	Clamp01(t);
	if (t < 0.5f)
	{
		return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * (t * t)));
	}
	else
	{
		float u = 2.0f * t - 2.0f;
		return 0.5f * (sqrtf(1.0f - (u * u)) + 1.0f);
	}
}

// ==================== Back（行き過ぎて戻る）====================
// 少しオーバーシュート（行き過ぎてから戻る）する特徴的な動き。
// ===============================================================

// 後ろに引いてから加速 overshootは最初に引く度合い
inline float EaseInBack(float t,float overshoot = EASE_BACK_OVERSHOOT)
{
	Clamp01(t);
	return t * t * ((overshoot + 1.0f) * t - overshoot);
}

// 行き過ぎてから戻る overshootは行き過ぎの度合い
inline float EaseOutBack(float t,float overshoot = EASE_BACK_OVERSHOOT)
{
	Clamp01(t);
	t -= 1.0f;
	return t * t * ((overshoot + 1.0f) * t + overshoot) + 1.0f;
}

// 前半は引いて加速、後半は行き過ぎて戻る overshootは度合い
inline float EaseInOutBack(float t,float overshoot = EASE_BACK_OVERSHOOT)
{
	Clamp01(t);
	float s = overshoot * 1.525f;

	if (t < 0.5f)
	{
		t *= 2.0f;
		return 0.5f * (t * t * ((s + 1.0f) * t - s));
	}
	else
	{
		t = t * 2.0f - 2.0f;
		return 0.5f * (t * t * ((s + 1.0f) * t + s) + 2.0f);
	}
}