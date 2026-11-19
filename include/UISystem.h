#pragma once
#include "vector2.h"

struct ButtonUIInfo
{
	bool isPressed{ false };	// 押している間true
	bool isTriggered{ false };	// 押した瞬間だけtrue
	bool isReleased{ false };	// 離した瞬間だけtrue

	bool isHovering{ false };	// マウスが乗っている間true
	bool isPressAndRelease;		// 押して離したときにtrueになる。ドラッグして離してもtrueになる。
};

struct SliderUIInfo
{
	float value{ 0.0f };	// スライダーの現在の値
	bool isDragging{ false };	// ドラッグしている間true
	bool isTriggered{ false };	// ドラッグを開始した瞬間だけtrue
	bool isReleased{ false };	// ドラッグを終了した瞬間だけtrue
	bool isHovering{ false };	// マウスが乗っている間true
};

void UISystemInitialize();
void UISystemFinalize();
void UISystemUpdate();
void UISystemDraw();

int CreateButton(
	const Vector2& position, 
	const Vector2& scale, 
	const float rotation, 
	int texture);

int CreateSlider(
	const Vector2& position,
	const Vector2& scale,
	const float rotation,
	int handleTexture,
	int barTexture,
	float minValue,
	float maxValue,
	float initialValue = 0.0f);

const ButtonUIInfo& GetButtonInfo(int id);
const SliderUIInfo& GetSliderInfo(int id);

const Vector2& GetScaledButtonSize(int id);
const Vector2& GetScaledSliderSize(int id);
