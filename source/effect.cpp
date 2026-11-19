#include "effect.h"
#include "directx.h"
#include "sprite.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"	

struct Effect
{
	bool active;
	Vector3 pos;
	int life;
	float scale;

	EffectType type;
};

Effect g_effects[100];

#define INT(x) static_cast<int>(x)

static constexpr int SPRITE_ROW[INT(EffectType::MAX)] =
{
	6,	// EXPLOSION
	6,	// EXPLOSION_BIG
};
static constexpr int SPRITE_COL[INT(EffectType::MAX)] =
{
	6,	// EXPLOSION
	6,	// EXPLOSION_BIG
};

static constexpr int SPRITE_FRAME[INT(EffectType::MAX)] =
{
	36,	// EXPLOSION
	36,	// EXPLOSION_BIG
};

static int g_texture[INT(EffectType::MAX)];
static ID3D11Buffer* g_vertexBuffer;

void EffectInitialize()
{
	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex) * 4; // 四角形分
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_vertexBuffer);
	}

	// テクスチャ読み込み
	g_texture[INT(EffectType::EXPLOSION)] = TextureLoad(L"asset\\texture\\expl_1.png");
	g_texture[INT(EffectType::EXPLOSION_BIG)] = TextureLoad(L"asset\\texture\\expl_1_1.png");

	for (int i = 0; i < 100; i++)
	{
		g_effects[i].active = false;
	}
}

void EffectFinalize()
{
	SAFE_RELEASE(g_vertexBuffer);
}

void EffectUpdate()
{
	for(Effect& effect : g_effects)
	{
		if(effect.active)
		{
			effect.life++;
			if (effect.life >= SPRITE_FRAME[INT(effect.type)])
			{
				effect.active = false;
			}
		}
	}
}

void EffectDraw()
{

	IsDepthEnable(true);


	// 頂点バッファセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Shader_Begin_3D();

	for (Effect& effect : g_effects)
	{
		if(effect.active)
		{

			int frame = effect.life;

			float tx = (frame % SPRITE_COL[INT(effect.type)]) / static_cast<float>(SPRITE_COL[INT(effect.type)]);
			float ty = (frame / SPRITE_COL[INT(effect.type)]) / static_cast<float>(SPRITE_ROW[INT(effect.type)]);
			float tw = 1.0f / static_cast<float>(SPRITE_COL[INT(effect.type)]);
			float th = 1.0f / static_cast<float>(SPRITE_ROW[INT(effect.type)]);

			// 頂点データ
			D3D11_MAPPED_SUBRESOURCE msr;
			DirectXGetDeviceContext()->Map(g_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			Vertex* v = (Vertex*)msr.pData;

			v[0].pos = XMFLOAT3(-0.5f, 0.5f, 0.0f);
			v[1].pos = XMFLOAT3(0.5f, 0.5f, 0.0f);
			v[2].pos = XMFLOAT3(-0.5f, -0.5f, 0.0f);
			v[3].pos = XMFLOAT3(0.5f, -0.5f, 0.0f);

			v[0].texcoord = XMFLOAT2(tx, ty);
			v[1].texcoord = XMFLOAT2(tx + tw, ty);
			v[2].texcoord = XMFLOAT2(tx, ty + th);
			v[3].texcoord = XMFLOAT2(tx + tw, ty + th);

			for(int i = 0; i < 4; i++)
			{
				v[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				v[i].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
			}

			DirectXGetDeviceContext()->Unmap(g_vertexBuffer, 0);

			Matrix matrix;

			matrix.world = XMMatrixIdentity();
			matrix.matrix = XMMatrixIdentity();

			// ビルボード行列の計算
			// ビュー行列の逆行列でビルボードは作れる
			XMMATRIX billBoardMatrix = XMMatrixInverse(nullptr, GetCameraViewMatrix());

			// 平行移動成分を削除
			// r→row(行),m128_f32→float配列
			billBoardMatrix.r[3].m128_f32[0] = 0.0f;
			billBoardMatrix.r[3].m128_f32[1] = 0.0f;
			billBoardMatrix.r[3].m128_f32[2] = 0.0f;

			matrix.world *= billBoardMatrix;
			matrix.world *= XMMatrixScaling(effect.scale, effect.scale, effect.scale);
			matrix.world *= XMMatrixTranslation(effect.pos.x, effect.pos.y, effect.pos.z);

			matrix.matrix = matrix.world;

			matrix.matrix *= GetCameraViewMatrix();

			matrix.matrix *= XMMatrixPerspectiveFovLH(1.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT,
				1.0f, 1000.0f);

			Shader_SetMatrix_3D(matrix);

			// テクスチャセット
			ID3D11ShaderResourceView* tex = GetTexture(g_texture[INT(effect.type)]);
			DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &tex);

			DirectXGetDeviceContext()->Draw(4, 0);
		}
	}
	IsDepthEnable(false);
}

void CreateEffect(const Vector3& pos, const float scale, EffectType type)
{
	for (Effect& effect : g_effects)
	{
		if (!effect.active)
		{
			effect.active = true;
			effect.pos = pos;
			effect.life = 0;
			effect.scale = scale;
			effect.type = type;
			break;
		}
	}
}
