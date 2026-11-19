#pragma once


#include "keyboard.h"
#include "mouse.h"
#include "utility.h"

enum MouseButton
{
	M_B_LEFT,	// 左クリック
	M_B_RIGHT,	// 右クリック
	M_B_MIDDLE	// ホイールクリック
};

// アップデートで呼ぶ（ゲームより先が望ましい）
void Mouse_Update();
// イニシャライズ用
void SetMouseState_double(Mouse_State m);

namespace Input
{
	// 押している間ずっとtrue
	// 引数:KK_～～ 例:KK_A
	inline bool Key_Down(Keyboard_Keys key) { return Keyboard_IsKeyDown(key); }
	// 離している間ずっとtrue
	// 引数:KK_～～ 例:KK_A
	inline bool Key_Up(Keyboard_Keys key) { return Keyboard_IsKeyUp(key); }
	// 押した瞬間だけtrue
	// 引数:KK_～～ 例:KK_A
	inline bool Key_Trigger(Keyboard_Keys key) { return Keyboard_IsKeyTrigger(key); }
	// 離した瞬間だけtrue
	// 引数:KK_～～ 例:KK_A
	inline bool Key_Release(Keyboard_Keys key) { return Keyboard_IsKeyRelease(key); }

	// 押している間ずっとtrue
	// 引数:M_B_～～ 例:M_B_LEFT
	bool Mouse_Down(MouseButton button);
	// 離している間ずっとtrue
	// 引数:M_B_～～ 例:M_B_LEFT
	bool Mouse_Up(MouseButton button);
	// 押した瞬間だけtrue
	// 引数:M_B_～～ 例:M_B_LEFT
	bool Mouse_Trigger(MouseButton button);
	// 離した瞬間だけtrue
	// 引数:M_B_～～ 例:M_B_LEFT
	bool Mouse_Release(MouseButton button);

	// スクロール量
	// 絶対量(合計値)
	int Mouse_Scroll();
	// スクロール量
	// フレーム間の差分
	int Mouse_DeltaScroll();

	// マウス座標
	// 絶対座標 現在の位置
	Vector2 Mouse_Pos();

	// マウス座標
	// 相対座標 フレーム間の差分
	Vector2 Mouse_DeltaPos();
}




