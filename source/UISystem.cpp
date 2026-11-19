#include "UISystem.h"
#include "utility.h"
#include <vector>
#include "input.h"
#include "texture.h"
#include "sprite.h"

static constexpr float HOVER_BRIGHTNESS = 0.85f;
static constexpr float PRESS_BRIGHTNESS = 0.7f;

struct ButtonUI
{
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;
	int texture;

	bool canPressAndRelease{ false };	// 押したあとドラッグしても離すまでtrue(info側の判定に使用)
};

struct SliderUI
{
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;
	int handleTexture;
	int barTexture;
	float sliderWidth;	// バーの幅(ピクセル)(barTextureの幅 * scale.x)
	float value;
	float minValue;
	float maxValue;

	Vector2 currentHandlePosition;	// ハンドルの現在の位置(ピクセル)
};

void ButtonUpdateProcess();
void SliderUpdateProcess();


std::vector<ButtonUI> g_buttons;
std::vector<SliderUI> g_sliders;
std::vector<ButtonUIInfo> g_buttonInfos;
std::vector<SliderUIInfo> g_sliderInfos;


void UISystemInitialize()
{
	g_buttons.clear();
	g_sliders.clear();
	g_buttonInfos.clear();
	g_sliderInfos.clear();
}

void UISystemFinalize()
{


}

void UISystemUpdate()
{
	ButtonUpdateProcess();
	SliderUpdateProcess();
}

void ButtonUpdateProcess()
{
	// ボタンの更新
	for (int i = 0; i < (int)g_buttons.size(); i++)
	{
		ButtonUI& button = g_buttons[i];
		ButtonUIInfo& info = g_buttonInfos[i];

		Vector2 buttonPixelSize =
			Vector2(TextureGetWidth(button.texture), TextureGetHeight(button.texture)) * Vector2(button.scale.x, button.scale.y);
		// ボタンの当たり判定や状態の更新をここで行う
		// マウス位置とボタンの位置・サイズを比較して、isHoveringやisPressedなどのフラグを更新する

		Vector2 mousePos = Input::Mouse_Pos();
		Vector2 buttonTopLeft = Vector2(button.position.x, button.position.y) - buttonPixelSize * 0.5f;
		Vector2 buttonBottomRight = buttonTopLeft + buttonPixelSize;

		info.isHovering = (mousePos.x >= buttonTopLeft.x && mousePos.x <= buttonBottomRight.x &&
			mousePos.y >= buttonTopLeft.y && mousePos.y <= buttonBottomRight.y);

		// 状態のリセット
		info.isTriggered = false;
		info.isPressed = false;
		info.isReleased = false;
		info.isPressAndRelease = false;

		// マウスの左ボタンが押されたときの処理
		if (Input::Mouse_Trigger(M_B_LEFT))
		{
			if (info.isHovering) info.isTriggered = true;
			button.canPressAndRelease = info.isHovering;
		}

		if (Input::Mouse_Release(M_B_LEFT))
		{
			if (info.isHovering) info.isReleased = true;
			if (button.canPressAndRelease) info.isPressAndRelease = true;
			button.canPressAndRelease = false;
		}

		if (Input::Mouse_Down(M_B_LEFT))
		{
			info.isPressed = button.canPressAndRelease;
		}
	}

}

void SliderUpdateProcess()
{
	// スライダーの更新
	for (int i = 0; i < (int)g_sliders.size(); i++)
	{
		SliderUI& slider = g_sliders[i];
		SliderUIInfo& info = g_sliderInfos[i];

		float t = InverseLerp(slider.minValue, slider.maxValue, slider.value);

		Vector2 sliderPixelSize =
			Vector2(TextureGetWidth(slider.barTexture), TextureGetHeight(slider.barTexture)) * Vector2(slider.scale.x, slider.scale.y);

		Vector2 handlePixelSize =
			Vector2(TextureGetWidth(slider.handleTexture), TextureGetHeight(slider.handleTexture)) * Vector2(slider.scale.x, slider.scale.y);

		Vector2 mousePos = Input::Mouse_Pos();
		Vector2 sliderTopLeft = Vector2(slider.position.x, slider.position.y) - sliderPixelSize * 0.5f;
		Vector2 sliderBottomRight = sliderTopLeft + sliderPixelSize;
		slider.currentHandlePosition = Vector2(slider.position.x, slider.position.y) - Vector2(slider.sliderWidth * 0.5f, 0.0f) + Vector2(t * slider.sliderWidth, 0.0f);
		Vector2 handleTopLeft = slider.currentHandlePosition - handlePixelSize * 0.5f;
		Vector2 handleBottomRight = handleTopLeft + handlePixelSize;

		// ハンドルの当たり判定
		info.isHovering = (mousePos.x >= handleTopLeft.x && mousePos.x <= handleBottomRight.x &&
			mousePos.y >= handleTopLeft.y && mousePos.y <= handleBottomRight.y);

		if (Input::Mouse_Trigger(M_B_LEFT))
		{
			if (info.isHovering) info.isTriggered = true;
			info.isDragging = info.isHovering;
		}
		else
		{
			info.isTriggered = false;
		}

		if (Input::Mouse_Release(M_B_LEFT))
		{
			if (info.isDragging) info.isReleased = true;
			info.isDragging = false;
		}
		else
		{
			info.isReleased = false;
		}

		if(Input::Mouse_Down(M_B_LEFT))
		{
			if(info.isDragging)
			{
				float newT = (mousePos.x - sliderTopLeft.x) / slider.sliderWidth;
				newT = Clamp(0.0f,1.0f,newT);
				slider.value = Lerp(slider.minValue, slider.maxValue, newT);
				info.value = slider.value;
			}
		}

	}
}

void UISystemDraw()
{
	for(int i = 0; i < (int)g_buttons.size(); i++)
	{
		ButtonUI& button = g_buttons[i];
		Vector2 buttonSize =
			Vector2(TextureGetWidth(button.texture), TextureGetHeight(button.texture));

		Color color = Color::WHITE();
		if (g_buttonInfos[i].isPressed) color = color.MulRGB(PRESS_BRIGHTNESS);
		else if (g_buttonInfos[i].isHovering) color = color.MulRGB(HOVER_BRIGHTNESS);

		SpriteDraw(button.position, buttonSize, button.scale, button.rotation.z, color, button.texture);
	}

	for(int i = 0; i < (int)g_sliders.size(); i++)
	{
		SliderUI& slider = g_sliders[i];
		SliderUIInfo& info = g_sliderInfos[i];
		Vector2 size =
			Vector2(TextureGetWidth(slider.barTexture), TextureGetHeight(slider.barTexture));

		Color color = Color::WHITE();
		SpriteDraw(slider.position, size, slider.scale, slider.rotation.z, color, slider.barTexture);

		size = Vector2(TextureGetWidth(slider.handleTexture), TextureGetHeight(slider.handleTexture));
		color = Color::WHITE();
		if(info.isDragging) color = color.MulRGB(PRESS_BRIGHTNESS);
		else if(info.isHovering) color = color.MulRGB(HOVER_BRIGHTNESS);
		SpriteDraw(Vector3(slider.currentHandlePosition.x, slider.currentHandlePosition.y, slider.position.z),
					size,
					slider.scale,
					slider.rotation.z,
					color,
					slider.handleTexture);

	}
}

int CreateButton(const Vector2& position, const Vector2& scale, const float rotation, int texture)
{
	ButtonUI button;
	button.position = Vector3(position.x, position.y, 0.0f);
	button.scale = Vector3(scale.x, scale.y, 1.0f);
	button.rotation = Vector3(0.0f, 0.0f, rotation);
	button.texture = texture;

	g_buttons.push_back(button);
	g_buttonInfos.push_back(ButtonUIInfo{});
	return (int)g_buttons.size() - 1;
}

int CreateSlider(const Vector2& position, const Vector2& scale, const float rotation, int handleTexture, int barTexture, float minValue, float maxValue, float initialValue)
{
	SliderUI slider;
	slider.position = Vector3(position.x, position.y, 0.0f);
	slider.scale = Vector3(scale.x, scale.y, 1.0f);
	slider.rotation = Vector3(0.0f, 0.0f, rotation);
	slider.handleTexture = handleTexture;
	slider.barTexture = barTexture;
	slider.minValue = minValue;
	slider.maxValue = maxValue;
	slider.value = initialValue;
	slider.sliderWidth = TextureGetWidth(barTexture) * scale.x;
	g_sliders.push_back(slider);

	SliderUIInfo sliderInfo;
	sliderInfo.value = initialValue;
	g_sliderInfos.push_back(sliderInfo);
	return (int)g_sliders.size() - 1;

}

const ButtonUIInfo& GetButtonInfo(int id)
{
	return g_buttonInfos[id];
}

const SliderUIInfo& GetSliderInfo(int id)
{
	return g_sliderInfos[id];
}

const Vector2& GetScaledButtonSize(int id)
{
	ButtonUI& button = g_buttons[id];
	Vector2 originalSize = Vector2(TextureGetWidth(button.texture), TextureGetHeight(button.texture));
	Vector2 scaledSize = originalSize * Vector2(button.scale.x, button.scale.y);
	return scaledSize;
}

const Vector2& GetScaledSliderSize(int id)
{
	SliderUI& slider = g_sliders[id];
	Vector2 originalSize = Vector2(TextureGetWidth(slider.barTexture), TextureGetHeight(slider.barTexture));
	Vector2 scaledSize = originalSize * Vector2(slider.scale.x, slider.scale.y);
	return scaledSize;
}