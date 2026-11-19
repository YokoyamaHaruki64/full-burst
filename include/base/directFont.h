#pragma once
#define NOMINMAX

#include "directx.h"
#include "utility.h"
#include <vector>
#include <string>

#include <dwrite_3.h>		// DirectWrite用
#include <d2d1.h>			// Direct2D用
#include <wrl/client.h>		// Microsoft::WRL::ComPtr用

#pragma comment(lib,"d2d1.lib")			// Direct2D用
#pragma comment(lib,"Dwrite.lib")		// DirectWrite用

void DirectFontInitialize(IDXGISwapChain* swapChain,HWND hWnd);
void DirectFontFinalize();
void WriteText(const std::string& text, Vector2 pos, Color textColor, float fontSize,
	int outlineWidth = 0, Color outlineColor = Color::WHITE());

IDWriteFactory5* GetDWriteFactory();