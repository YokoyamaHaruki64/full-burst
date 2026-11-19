#pragma once

#define NOMINMAX	//windows.hの前
#include <Windows.h>
#include <d3d11.h>	//DirectX関係(Direct3D11の略)
#include <DirectXMath.h>
#include "utility.h"

using namespace DirectX;

enum BLEND_STATE
{
	NOT_ALPHA,
	ALPHABLEND,
	ADD,
};

#define SAFE_RELEASE(o)	if(o){	(o)->Release(); o = NULL;}
#define WINDOW_WIDTH	1600
#define WINDOW_HEIGHT	900

#define BLOCK_ROWS 6
#define BLOCK_COLS 13

void DirectXInitialize(HWND hWnd);
void DirectXFinalaize();

ID3D11Device* DirectXGetDevice();	//DirectXのコントロールに使う物を取得
ID3D11DeviceContext* DirectXGetDeviceContext();

void Clear();	//画面クリア
void Present();

void ShadowBegin();
void ShadowEnd();

void SetClearColor(Color color);
void SetBlendState(BLEND_STATE state);

void IsDepthEnable(bool enable);
void IsDepthWriteEnable(bool enable);

ID3D11ShaderResourceView* const* GetShadowMapSRV();

void CreateResultSnapshot();
ID3D11ShaderResourceView* const* GetResultSnapshotSRV();


