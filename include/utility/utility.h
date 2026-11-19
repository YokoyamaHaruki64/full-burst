#pragma once
#define _USE_MATH_DEFINES

#include <algorithm> 
#include <cmath>     
#include <DirectXMath.h>

#include "libMath.h"
#include "vector2.h"
#include "vector3.h"
#include "libTime.h"
#include "frameTime.h"
#include "ease.h"
#include "random.h"

//=========================================================
// イージング関数
//
// t : 0.0f ～ 1.0f の範囲の時間（進行度）を表す。
//     0 → 開始, 1 → 終了
//
// 戻り値 : 同じく 0.0f ～ 1.0f の範囲の補間値を返す。
//     値の増え方が直線的ではなく、滑らか・自然な動きになる。
//
// EaseIn  　: 最初はゆっくり、最後は速く（加速する動き）
// EaseOut 　: 最初は速く、最後はゆっくり（減速する動き）
// EaseInOut : 最初と最後がゆっくり、中間が速い（自然な加減速）
// 
// InverseLerpで0～1に変換できるので相性が〇
// 
// カーブの参照 - https://easings.net/ja
//=========================================================


struct Color
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    constexpr Color() = default;
    constexpr Color(float red, float green, float blue, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}

    // DirectX::XMFLOAT4 に変換
    DirectX::XMFLOAT4 ToXMFLOAT4() const
    {
        return DirectX::XMFLOAT4(r, g, b, a);
    }

    static Color FromXMFLOAT4(const DirectX::XMFLOAT4& xf4)
    {
        return Color(xf4.x, xf4.y, xf4.z, xf4.w);
    }

// ------------------演算子------------------
// ---基本演算子---
    constexpr Color operator+(const Color& rhs) const 
    {
        return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
    }

    constexpr Color operator-(const Color& rhs) const 
    {
        return Color(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
    }

    constexpr Color operator*(const float s) const
    {
        return Color(r * s, g * s, b * s, a * s);
    }

    constexpr Color operator/(const float s) const
    {
        return Color(r / s, g / s, b / s, a / s);
    }

    constexpr Color operator*(const Color& rhs) const 
    {
        return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
    }

    constexpr Color operator/(const Color& rhs) const 
    {
        return Color(r / rhs.r, g / rhs.g, b / rhs.b, a / rhs.a);
    }

    // --- 複合代入演算子 ---
    Color& operator+=(const Color& rhs) 
    {
        return *this = *this + rhs;
    }

    Color& operator-=(const Color& rhs) 
    {
        return *this = *this - rhs;
    }

    Color& operator*=(const float s) 
    {
        return *this = *this * s;
    }

    Color& operator/=(const float s) 
    {
        return *this = *this / s;
    }

    Color& operator*=(const Color& rhs) 
    {
        return *this = *this * rhs;
    }

    Color& operator/=(const Color& rhs) 
    {
        return *this = *this / rhs;
    }

    constexpr Color AddRGB(const Color& rhs) const
    {
        return Color(r + rhs.r, g + rhs.g, b + rhs.b, a);
    }

    constexpr Color AddRGB(const float s) const
    {
        return Color(r + s, g + s, b + s, a);
    }

    constexpr Color SubRGB(const Color& rhs) const
    {
        return Color(r - rhs.r, g - rhs.g, b - rhs.b, a);
    }

    constexpr Color SubRGB(const float s) const
    {
        return Color(r - s, g - s, b - s, a);
    }

    constexpr Color MulRGB(const Color& rhs) const
    {
        return Color(r * rhs.r, g * rhs.g, b * rhs.b, a);
    }

    constexpr Color MulRGB(const float s) const
    {
        return Color(r * s, g * s, b * s, a);
    }

    constexpr Color DivRGB(const Color& rhs) const
    {
        return Color(r / rhs.r, g / rhs.g, b / rhs.b, a);
    }

    constexpr Color DivRGB(const float s) const
    {
        return Color(r / s, g / s, b / s, a);
    }

    // --- 比較演算 ---
    bool operator==(const Color& rhs) const 
    {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }

    bool operator!=(const Color& rhs) const 
    {
        return !(*this == rhs);
    }
    // --- Clamp（範囲制限） ---
    void ColorClamp()
    {
        r = Clamp01(r);
        g = Clamp01(g);
        b = Clamp01(b);
        a = Clamp01(a);
    }

    // --- 線形補間（Lerp） ---
    static constexpr Color Lerp(const Color& c1, const Color& c2, float t) 
    {
        return Color
        (
            c1.r + (c2.r - c1.r) * t,
            c1.g + (c2.g - c1.g) * t,
            c1.b + (c2.b - c1.b) * t,
            c1.a + (c2.a - c1.a) * t
        );
    }

	/// @brief HSVからRGBへ変換
	/// @param h hue値 (0～360)
	/// @param s saturation値 (0～100)
	/// @param v value値 (0～100)
	/// @param a alpha値 (0.0f～1.0f)
	/// @return 変換後のColor値 (RGB)
    static Color HSVtoRGB(int hue, int saturation, int value, float alpha)
    {
		float h = static_cast<float>(hue);
		float s = static_cast<float>(saturation) / 100.0f;
		float v = static_cast<float>(value) / 100.0f;

        s = Clamp(0.0f, 1.0f, s);
        v = Clamp(0.0f, 1.0f, v);

        float r = 0, g = 0, b = 0;

        if (s <= 0.0f)
        {
            r = g = b = v; // 無彩色
        }
        else
        {
            h = Fmod(h, 360.0f);
            if (h < 0)
            {
                h += 360.0f;
            }
            float hf = h / 60.0f;
            int i = static_cast<int>(hf);
            float f = hf - i;
            float p = v * (1.0f - s);
            float q = v * (1.0f - s * f);
            float t = v * (1.0f - s * (1 - f));

            switch (i % 6)
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
                r = v;
                g = p;
                b = q;
                break;
            }
        }

        return Color(r, g, b, alpha);
    }

    // static constexpr定義
    static constexpr Color WHITE() { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
    static constexpr Color BLACK() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
    static constexpr Color RED() { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
    static constexpr Color GREEN() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
    static constexpr Color BLUE() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }

};

struct UV
{
    float u1 = 0.0f;
    float v1 = 0.0f;
    float u2 = 1.0f;
    float v2 = 1.0f;

	bool isFlipX = false;
	bool isFlipY = false;

    constexpr UV() = default;
    constexpr UV(float _u1 = 0.0f, float _v1 = 0.0f, float _u2 = 1.0f, float _v2 = 1.0f)
        : u1(_u1), v1(_v1), u2(_u2), v2(_v2) {}

// ----------------------------------------------
// 基本情報取得
// ----------------------------------------------
    constexpr float U2() const { return u1 + u2; }// uの右側位置
    constexpr float V2() const { return v1 + v2; }// vの下側位置
    constexpr float Width() const { return u2; }
    constexpr float Height() const { return v2; }

// ----------------------------------------------
// 操作関数
// ----------------------------------------------

    // du,dyの分それぞれずらしたUVを返す
    constexpr UV Offset(float du, float dv) const 
    {
        return UV(u1 + du, v1 + dv, u2, v2);
    }

    // 左右反転したUVを返す
    constexpr UV FlippedX() const 
    {
        return UV(U2(), v1, -u2, v2);
    }

    // 上下反転したUVを返す
    constexpr UV FlippedY() const 
    {
        return UV(u1, V2(), u2, -v2);
    }

    constexpr void FlipX() 
    {
        u1 = U2();
        u2 = -u2;
		isFlipX = !isFlipX;
	}

    constexpr void FlipY()
    {
        v1 = V2();
        v2 = -v2;
		isFlipY = !isFlipY;
    }
    // ----------------------------------------------
    // スプライトシート関連
    // ----------------------------------------------

    // @brief スプライトシートの情報からUV領域を作成
    // @param cols スプライトの列数（横方向の分割数）
    // @param rows スプライトの行数（縦方向の分割数）
    // @param index コマ番号（0～cols*rows-1）
    // @return 指定コマに対応するUV値(u,v,width,height)
    // 
    // 例 renderInfo.uv = UV::FromSheet(4,5,3)
    static constexpr UV FromSheet(int cols, int rows, int index)
    {
        float tileW = 1.0f / cols;
        float tileH = 1.0f / rows;

        int x = index % cols; // 横方向の位置
        int y = index / cols; // 縦方向の位置

        float u = tileW * x;
        float v = tileH * y;

        return UV(u, v, tileW, tileH);
    }
};
