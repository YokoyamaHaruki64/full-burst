#pragma once
#undef min
#undef max

#include <math.h>
#include <algorithm>
#undef min
#undef max

constexpr float M_2PI = 3.14159265358979323846f * 2.0f;		// 2π
constexpr float M_HALF_PI = 3.14159265358979323846f * 0.5f;	// π/2
constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;	// 角度をラジアンに変換する定数
constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;	// ラジアンを角度に変換する定数

// 平方根
inline float Sqrt(float x) { return sqrtf(x); }	

// 絶対値
inline float Abs(float x) { return fabsf(x); }

// サイン
inline float Sin(float x) { return sinf(x); }	

// コサイン
inline float Cos(float x) { return cosf(x); }	

// タンジェント
inline float Tan(float x) { return tanf(x); }	

// アークサイン
inline float Asin(float x) { return asinf(x); }	

// アークコサイン
inline float Acos(float x) { return acosf(x); }	

// アークタンジェント
inline float Atan(float x) { return atanf(x); }	

// アークタンジェント(y/x) 
inline float Atan2(float y, float x) { return atan2f(y, x); }	

//	xのy乗 重いので注意
inline float Pow(float x, float y) { return powf(x, y); }

// 自然対数の底 e の x 乗
inline float Exp(float x) { return expf(x); }	

// 自然対数
inline float Log(float x) { return logf(x); }	

// 切り捨て
inline float Floor(float x) { return floorf(x); }	

// 切り上げ
inline float Ceil(float x) { return ceilf(x); }	

// 四捨五入
inline float Round(float x) { return roundf(x); }	

// aをbで割った余り(float版)
inline float Fmod(float a, float b) { return fmodf(a, b); }	

// 角度をラジアンに変換
inline float DegreeToRadian(float degree) { return degree * DEG_TO_RAD; }	

// ラジアンを角度に変換
inline float RadianToDegree(float radian) { return radian * RAD_TO_DEG; }

// ===========================================
// Clamp（値を範囲内に制限）
// ===========================================
template <typename T>
inline T Clamp(const T& min, const T& max, const T& value)
{
    return std::min(max, std::max(min, value));
}

// ===========================================
// Clamp01（0～1に制限）
// ===========================================
template <typename T>
inline T Clamp01(const T& value)
{
    return Clamp(T(0), T(1), value);
}

// ===========================================
// Lerp（線形補間）
// -------------------------------------------
// begin から end まで、t（0～1）の割合で間を取る。
// 例）Lerp(0,10,0.5) → 5
// ===========================================
inline float Lerp(const float& begin, const float& end, float t)
{
    t = Clamp01(t);
    return begin + (end - begin) * t;
}

inline float LerpUnclamped(const float& begin, const float& end, float t)
{
    return begin + (end - begin) * t;
}


// ===========================================
// InverseLerp（逆線形補間）
// -------------------------------------------
// begin と end の間で、value がどの位置にあるかを 0～1 で返す。
// 例）InverseLerp(0,10,5) → 0.5
// ===========================================
inline float InverseLerp(const float& begin, const float& end, const float& value)
{
    if (begin == end) return 0.0f;
    return (value - begin) / (end - begin);
}

// ===========================================
// Remap（範囲の変換）
// -------------------------------------------
// ある範囲 [inMin, inMax] の値を、
// 別の範囲 [outMin, outMax] に変換する。
// 例）Remap(0,100,0,1,50) → 0.5
// ===========================================
inline float Remap(const float& inMin, const float& inMax, const float& outMin, const float& outMax, const float& value)
{
    float t = InverseLerp(inMin, inMax, value);
    return Lerp(outMin, outMax, t);
}

// ===========================================
// SmoothStep（なめらかな補間）
// -------------------------------------------
// 0～1の範囲を滑らかに補間する。
// Lerpよりも「加速→減速」するカーブを描く。
// ===========================================
inline float SmoothStep(float edge0, float edge1, float x)
{
    float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3 - 2 * t);
}

// ===========================================
// Sign（符号を返す）
// -------------------------------------------
// 正なら +1、負なら -1、0なら 0 を返す。
// ===========================================
template <typename T>
inline int Sign(const T& value)
{
    return (value > 0) - (value < 0);
}

// ===========================================
// AlmostEqual（ほぼ等しいか？）
// -------------------------------------------
// 浮動小数点の誤差を考慮して「ほぼ同じ」を判定。
// ===========================================
inline bool AlmostEqual(float a, float b, float epsilon = 1e-6f)
{
    return abs(a - b) <= epsilon;
}

// ===========================================
// PingPong（往復する値）
// -------------------------------------------
// t の値を「0→length→0→length…」と行き来させる。
// 例）PingPong(100, 130) → 70
// ===========================================
inline float PingPong(float length, float t)
{
    t = Fmod(t, length * 2);
    return length - abs(t - length);
}

// ===========================================
// Repeat（繰り返し）
// -------------------------------------------
// t を 0～length の範囲でループさせる。
// 例）Repeat(7,5) → 2
// ===========================================
inline float Repeat(float t, float length)
{
    return t - Floor(t / length) * length;
}
