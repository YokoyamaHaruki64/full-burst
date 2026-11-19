#pragma once
#include <DirectXMath.h>
#include <vector>
#include "utility.h"
using namespace DirectX;

// 頂点構造体
//XMFLOAT4は名前空間DirectXである(stdと同じ感じ、using namespaceで省略可)
struct Vertex
{
	XMFLOAT3 pos;	// 頂点座標
	XMFLOAT4 color;		// 色
	XMFLOAT2 texcoord;	// テクスチャ座標
	XMFLOAT3 normal;	// テクスチャ座標
};

struct VertexShadow
{
	XMFLOAT3 pos;	// 頂点座標
};

void SpriteInitialize();
void SpriteFinalize();
void SpriteDraw(Vector3 pos, Vector3 size, Vector3 scale, float rot, Color color, int texID,
				float u = 0.0f, float V = 0.0f, float uw = 1.0f, float vh = 1.0f);

void SpriteDraw(Vector3 pos, Vector3 size, Vector3 scale, float rot, Color color,
	ID3D11ShaderResourceView* const* texture, 
	float u = 0.0f, float V = 0.0f, float uw = 1.0f, float vh = 1.0f);

void BillboardDraw(Vector3 pos, Vector3 scale, float rot, Color color, int texID, 
	Vector3 offset = {0.0f,0.0f,0.0f}, float u = 0.0f, float V = 0.0f, float uw = 1.0f, float vh = 1.0f);

