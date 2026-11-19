#include "input.h"
// マウス関連のものはDownなどがなかったのでここで簡易的に実装する

struct FrameMouseData
{
	// 左クリックの判定
	bool leftUp		 = false;
	bool leftDown	 = false;
	bool leftTrigger = false;
	bool leftRelease = false;

	// 中クリックの判定
	bool middleUp		= false;
	bool middleDown		= false;
	bool middleTrigger	= false;
	bool middleRelease	= false;

	// 右クリックの判定
	bool rightUp		= false;
	bool rightDown		= false;
	bool rightTrigger	= false;
	bool rightRelease	= false;

	// スクロール
	int scroll = 0;
	int deltaScroll = 0;

	// 絶対座標用
	Vector2 mousePos = {};

	// 絶対座標用
	Vector2 deltaMouse = {};
};

static FrameMouseData g_data{};
static Mouse_State g_mouseState;
static Mouse_State g_oldMouseState;

void Mouse_Update()
{
	Mouse_GetState(&g_mouseState);

	// 左クリ
	g_data.leftDown = g_mouseState.leftButton;
	g_data.leftUp = !g_mouseState.leftButton;
	g_data.leftTrigger = (g_mouseState.leftButton && !g_oldMouseState.leftButton);
	g_data.leftRelease = (!g_mouseState.leftButton && g_oldMouseState.leftButton);

	// 右クリ
	g_data.rightDown = g_mouseState.rightButton;
	g_data.rightUp = !g_mouseState.rightButton;
	g_data.rightTrigger = (g_mouseState.rightButton && !g_oldMouseState.rightButton);
	g_data.rightRelease = (!g_mouseState.rightButton && g_oldMouseState.rightButton);

	// 右クリ
	g_data.middleDown = g_mouseState.middleButton;
	g_data.middleUp = !g_mouseState.middleButton;
	g_data.middleTrigger = (g_mouseState.middleButton && !g_oldMouseState.middleButton);
	g_data.middleRelease = (!g_mouseState.middleButton && g_oldMouseState.middleButton);

	// ホイール取得
	g_data.scroll = g_mouseState.scrollWheelValue;
	g_data.deltaScroll = g_mouseState.scrollWheelValue - g_oldMouseState.scrollWheelValue;

	// マウス位置取得
	g_data.mousePos.x = (float)g_mouseState.x;
	g_data.mousePos.y = (float)g_mouseState.y;

	// 差分取得
	g_data.deltaMouse.x = (float)(g_mouseState.x - g_oldMouseState.x);
	g_data.deltaMouse.y = (float)(g_mouseState.y - g_oldMouseState.y);

	g_oldMouseState = g_mouseState;
}

void SetMouseState_double(Mouse_State m)
{
	g_mouseState = m;
	g_oldMouseState = m;
}

namespace Input
{

	bool Mouse_Down(MouseButton button)
	{
		switch (button)
		{
		case M_B_LEFT:
			return g_data.leftDown;
			break;

		case M_B_RIGHT:
			return g_data.rightDown;
			break;

		case M_B_MIDDLE:
			return g_data.middleDown;
			break;

		default:
			return false;
			break;
		}
	}

	bool Mouse_Up(MouseButton button)
	{
		switch (button)
		{
		case M_B_LEFT:
			return g_data.leftUp;
			break;

		case M_B_RIGHT:
			return g_data.rightUp;
			break;

		case M_B_MIDDLE:
			return g_data.middleUp;
			break;

		default:
			return false;
			break;
		}
	}

	bool Mouse_Trigger(MouseButton button)
	{
		switch (button)
		{
		case M_B_LEFT:
			return g_data.leftTrigger;
			break;

		case M_B_RIGHT:
			return g_data.rightTrigger;
			break;

		case M_B_MIDDLE:
			return g_data.middleTrigger;
			break;

		default:
			return false;
			break;
		}
	}

	bool Mouse_Release(MouseButton button)
	{
		switch (button)
		{
		case M_B_LEFT:
			return g_data.leftRelease;
			break;

		case M_B_RIGHT:
			return g_data.rightRelease;
			break;

		case M_B_MIDDLE:
			return g_data.middleRelease;
			break;

		default:
			return false;
			break;
		}
	}

	int Mouse_Scroll()
	{
		return g_data.scroll;
	}

	int Mouse_DeltaScroll()
	{
		return g_data.deltaScroll;
	}

	Vector2 Mouse_Pos()
	{
		return g_data.mousePos;
	}

	Vector2 Mouse_DeltaPos()
	{
		return g_data.deltaMouse;
	}

}




