#pragma once
#undef min
#undef max

#include <math.h>
#include <algorithm>
#include <DirectXMath.h>
#include "libMath.h"

struct Vector3;

struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;
    
    // ------------------ コンストラクタ ------------------
    constexpr Vector2() {};
    constexpr Vector2(float x) : x(x), y(0.0f) {}   // 引数ないものは0初期化
    constexpr Vector2(float x, float y) : x(x), y(y) {}
    
	Vector2(const Vector3& vec3);

    // ------------------ プロパティ ------------------
	inline float Length() const { return Sqrt(x * x + y * y); } // 長さ
	inline float SqrLength() const { return x * x + y * y; }    // 長さの二乗
    
	void Normalize(float length);   // 自分自身を正規化
	void Normalize();   // 自分自身を正規化
	Vector2 Normalized() const; // 正規化した新しいベクトルを返す
	Vector2 Normalized(float length) const; // 長さ指定で正規化した新しいベクトルを返す
    inline void Set(float newX, float newY) { x = newX; y = newY; }
    
    // ------------------ static 常用ベクトル ------------------
    inline static constexpr Vector2 Zero() { return { 0,0 }; }
    inline static constexpr Vector2 One() { return { 1,1 }; }
    inline static constexpr Vector2 Up() { return { 0,1 }; }
    inline static constexpr Vector2 Down() { return { 0,-1 }; }
    inline static constexpr Vector2 Left() { return { -1,0 }; }
    inline static constexpr Vector2 Right() { return { 1,0 }; }
    
    // ------------------ 演算子 ------------------
    inline constexpr Vector2 operator+(const Vector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
    inline constexpr Vector2 operator-(const Vector2& rhs) const { return { x - rhs.x, y - rhs.y }; }
    inline constexpr Vector2 operator*(const Vector2& rhs) const { return { x * rhs.x, y * rhs.y }; }
    inline constexpr Vector2 operator/(const Vector2& rhs) const { return { x / rhs.x, y / rhs.y }; }
    inline constexpr Vector2 operator*(float s) const { return { x * s, y * s }; }
    inline constexpr Vector2 operator/(float s) const { return { x / s, y / s }; }
    
    inline Vector2& operator+=(const Vector2& rhs) { return *this = *this + rhs; }
	inline Vector2& operator-=(const Vector2& rhs) { return *this = *this - rhs; }
	inline Vector2& operator*=(const Vector2& rhs) { return *this = *this * rhs; }
	inline Vector2& operator/=(const Vector2& rhs) { return *this = *this / rhs; }
	inline Vector2& operator*=(float s) { return *this = *this * s; }
	inline Vector2& operator/=(float s) { return *this = *this / s; }
    
    inline constexpr bool operator==(const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }
    inline constexpr bool operator!=(const Vector2& rhs) const { return !(*this == rhs); }
    
	// ------------------ Vector3との演算 ------------------
	Vector2 operator+(const Vector3& rhs) const;
	Vector2 operator-(const Vector3& rhs) const;
	Vector2 operator*(const Vector3& rhs) const;
	Vector2 operator/(const Vector3& rhs) const;

    Vector2& operator+=(const Vector3& rhs);
	Vector2& operator-=(const Vector3& rhs);
	Vector2& operator*=(const Vector3& rhs);
	Vector2& operator/=(const Vector3& rhs);

	// ------------------ Vector3代入 ------------------
	Vector2& operator=(const Vector3& rhs);

	// ------------------ XMFLOAT2変換 ------------------
    inline DirectX::XMFLOAT2 ToXMFLOAT2() const
    {
        return DirectX::XMFLOAT2(x, y);
    }
    inline static Vector2 FromXMFLOAT2(const DirectX::XMFLOAT2& v)
    {
        return Vector2(v.x, v.y);
	}

    // ------------------ static ユーティリティ ------------------
    inline static float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }
	inline float Dot(const Vector2& b) const { return x * b.x + y * b.y; } // 自分自身との内積
    
    static float Distance(const Vector2& a, const Vector2& b);
    static float SqrDistance(const Vector2& a, const Vector2& b);
    
    inline static Vector2 Lerp(const Vector2& a, const Vector2& b, float t)
    {
        t = Clamp01(t);
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
    }
    
    inline static Vector2 LerpUnclamped(const Vector2& a, const Vector2& b, float t)
    {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
    }
    
	// 新しいスケールドベクトルを返す
    inline static Vector2 Scaled(const Vector2& a, const Vector2& b) 
    {
        return { a.x * b.x, a.y * b.y };
    }
    
	// 自分自身をスケールドベクトルで拡大縮小する
    inline void Scale(const Vector2& b) { x *= b.x; y *= b.y; }
    
    inline static Vector2 Min(const Vector2& a, const Vector2& b) 
    {
        return { std::min(a.x,b.x), std::min(a.y,b.y) };
    }
    
    inline static Vector2 Max(const Vector2& a, const Vector2& b) 
    {
        return { std::max(a.x,b.x), std::max(a.y,b.y) };
    }
    
    // ------------------ 回転 ------------------
	void Rotate(float degrees);  // 自分自身を回転させる
	Vector2 Rotated(float degrees) const;   // 回転させた新しいベクトルを返す
};