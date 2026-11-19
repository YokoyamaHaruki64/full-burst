#pragma once
#undef min
#undef max

#include <math.h>
#include <algorithm>
#include <DirectXMath.h>
#include "libMath.h"

struct Vector2;

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // ------------------ コンストラクタ ------------------
    constexpr Vector3() {}
    constexpr Vector3(float x) : x(x), y(0.0f), z(0.0f) {}// 引数ないのは0初期化
    constexpr Vector3(float x, float y) : x(x), y(y), z(0.0f) {}// 引数ないのは0初期化
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3(const Vector2& vec2);

    // ------------------ 定数ベクトル ------------------
    static constexpr Vector3 Zero() { return Vector3(0, 0, 0); }
    static constexpr Vector3 One() { return Vector3(1, 1, 1); }
    static constexpr Vector3 Up() { return Vector3(0, 1, 0); }
    static constexpr Vector3 Down() { return Vector3(0, -1, 0); }
    static constexpr Vector3 Left() { return Vector3(-1, 0, 0); }
    static constexpr Vector3 Right() { return Vector3(1, 0, 0); }
    static constexpr Vector3 Forward() { return Vector3(0, 0, 1); }
    static constexpr Vector3 Back() { return Vector3(0, 0, -1); }

    // ------------------ 演算子 ------------------
    inline constexpr Vector3 operator+(const Vector3& rhs) const { return Vector3{ x + rhs.x, y + rhs.y, z + rhs.z }; }
    inline constexpr Vector3 operator-(const Vector3& rhs) const { return Vector3{ x - rhs.x, y - rhs.y, z - rhs.z }; }

    inline constexpr Vector3 operator*(const Vector3& rhs) const { return Vector3{ x * rhs.x, y * rhs.y, z * rhs.z }; }
    inline constexpr Vector3 operator/(const Vector3& rhs) const { return Vector3{ x / rhs.x, y / rhs.y, z / rhs.z }; }

    inline constexpr Vector3 operator*(const float rhs) const { return Vector3{ x * rhs, y * rhs, z * rhs }; }
    inline constexpr Vector3 operator/(const float rhs) const { return Vector3{ x / rhs, y / rhs, z / rhs }; }

    inline Vector3& operator+=(const Vector3& rhs) { return *this = *this + rhs; }
    inline Vector3& operator-=(const Vector3& rhs) { return *this = *this - rhs; }

    inline Vector3& operator*=(const Vector3& rhs) { return *this = *this * rhs; }
    inline Vector3& operator/=(const Vector3& rhs) { return *this = *this / rhs; }

    inline Vector3& operator*=(float rhs) { return *this = *this * rhs; }
    inline Vector3& operator/=(float rhs) { return *this = *this / rhs; }

    inline bool constexpr operator==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    inline bool constexpr operator!=(const Vector3& rhs) const { return !(*this == rhs); }

	// Vector2との演算
	Vector3 operator+(const Vector2& rhs) const;
	Vector3 operator-(const Vector2& rhs) const;
	Vector3 operator*(const Vector2& rhs) const;
	Vector3 operator/(const Vector2& rhs) const;

	Vector3& operator+=(const Vector2& rhs);
	Vector3& operator-=(const Vector2& rhs);
	Vector3& operator*=(const Vector2& rhs);
	Vector3& operator/=(const Vector2& rhs);

    // Vector3に代入
    Vector3& operator=(const Vector2& rhs);

	// XMFLOAT3 に変換
    inline DirectX::XMFLOAT3 ToXMFLOAT3() const 
    {
        return DirectX::XMFLOAT3(x, y, z);
	}

	// XMFLOAT3 から変換
    inline static Vector3 FromXMFLOAT3(const DirectX::XMFLOAT3& v) 
    {
		return Vector3(v.x, v.y, v.z);
	}

    // ------------------ ベクトル演算 ------------------
    inline float Length() const { return sqrtf(x * x + y * y + z * z); }
    inline float SqrLength() const { return x * x + y * y + z * z; }

	// 自分自身を正規化
    void Normalize();

	// 正規化した新しいベクトルを返す
    Vector3 Normalized() const;

    inline static float Dot(const Vector3& a, const Vector3& b) 
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

	// 自分自身との内積
    // 引数はa・bのbの方
    inline float Dot(const Vector3& other) const 
    {
        return x * other.x + y * other.y + z * other.z;
	}

    inline static Vector3 Cross(const Vector3& a, const Vector3& b) 
    {
        return Vector3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

	// 自分自身との外積
    inline Vector3 Cross(const Vector3& other) const 
    {
        return Vector3{
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
	}

    // ------------------ 距離・補間 ------------------
    static float Distance(const Vector3& a, const Vector3& b);

    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) 
    {
        t = Clamp01(t);
        return a + (b - a) * t;
    }

    static Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta);

    // ------------------ 回転・角度関連 ------------------
    static float Angle(const Vector3& from, const Vector3& to);

    inline static Vector3 Reflect(const Vector3& inDir, const Vector3& inNormal) 
    {
        return inDir - inNormal * (2.0f * Dot(inDir, inNormal));
    }

    // ------------------ 便利関数 ------------------
    inline bool IsZero() const { return x == 0 && y == 0 && z == 0; }
};

