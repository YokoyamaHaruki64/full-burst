#include "B_verticalTarget.h"
#include "B_verticalLaser.h"
#include <vector>
#include "utility.h"
#include "directx.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "sprite.h"
#include "collision.h"

#include "field.h"
#include "player.h"

struct Target
{
	Vector3 position;
	FrameTime* lifeTime;
	bool isActive;
	float radius;
};


static Target g_target[MAX_VERTICAL_LASER];

static int g_targetTex;
static ID3D11Buffer* g_vertexBuffer;
static float dt = 0.016f;

void VerticalTargetInitialize()
{
	g_targetTex = TextureLoad(L"asset\\texture\\target.png");

	for(int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		g_target[i].isActive = false;
		g_target[i].lifeTime = new FrameTime(60);
	}

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_vertexBuffer);

	// 頂点データ
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* v = (Vertex*)msr.pData;

	v[0].pos = XMFLOAT3(-0.5f, 0.0f, 0.5f);
	v[1].pos = XMFLOAT3(0.5f, 0.0f, 0.5f);
	v[2].pos = XMFLOAT3(-0.5f, 0.0f, -0.5f);
	v[3].pos = XMFLOAT3(0.5f, 0.0f, -0.5f);

	v[0].texcoord = XMFLOAT2(0.0f, 0.0f);
	v[1].texcoord = XMFLOAT2(1.0f, 0.0f);
	v[2].texcoord = XMFLOAT2(0.0f, 1.0f);
	v[3].texcoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		v[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		v[i].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}

	DirectXGetDeviceContext()->Unmap(g_vertexBuffer, 0);

}

void VerticalTargetFinalize()
{
	SAFE_RELEASE(g_vertexBuffer);
	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		g_target[i].isActive = false;
		delete g_target[i].lifeTime;
	}

}

void VerticalTargetUpdate()
{
	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_target[i].isActive == false)
		{
			continue;
		}

		if (g_target[i].lifeTime->IsFinished())
		{
			g_target[i].isActive = false;
			Vector3 pos = g_target[i].position;
			pos.y += 50.0f;
			if (GetPlayerState() != PlayerState::CHARGE &&
				GetPlayerState() != PlayerState::SHOT &&
				GetPlayerState() != PlayerState::RETURN)
			{
				CreateVerticalLaser(pos, g_target[i].radius);
			}
		}
	}
}

void VerticalTargetDraw()
{
	IsDepthEnable(true);

	Rimlight rim;
	rim.cameraPos = GetCameraPos().ToXMFLOAT3();
	rim.color = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Shader_SetRimlight(rim);

	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_target[i].isActive == false)
		{
			continue;
		}
		Shader_Begin_3D();

		Matrix matrix;

		matrix.world = XMMatrixIdentity();
		matrix.matrix = XMMatrixIdentity();

		matrix.world *= XMMatrixScaling(g_target[i].radius * 2.0f, 1.0f, g_target[i].radius * 2.0f);
		matrix.world *= XMMatrixTranslation(
			g_target[i].position.x,
			g_target[i].position.y,
			g_target[i].position.z
		);

		matrix.matrix = matrix.world;

		matrix.matrix *= GetCameraViewMatrix();
		matrix.matrix *= GetProjectionMatrix();

		Shader_SetMatrix_3D(matrix);

		// テクスチャセット
		ID3D11ShaderResourceView* tex = GetTexture(g_targetTex);
		DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &tex);

		// 頂点バッファセット
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

		// プリミティブトポロジ設定
		DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		DirectXGetDeviceContext()->Draw(4, 0);
	}
	IsDepthEnable(false);

}

Vector3 CheckTargetOnField(Vector3 pos)
{
	std::vector<Block> block = GetBlocks();

	float targetY = FLT_MIN;

	for (int i = 0; i < (int)block.size(); i++)
	{
		XMFLOAT3 blockPos = block[i].position;
		XMFLOAT3 blockScale = block[i].scale;

		if (pos.z > blockPos.z - blockScale.z * 0.5f &&
			pos.z < blockPos.z + blockScale.z * 0.5f)
		{
			if (pos.x > blockPos.x - blockScale.x * 0.5f &&
				pos.x < blockPos.x + blockScale.x * 0.5f)
			{
				if (pos.y > blockPos.y - blockScale.y * 0.5f)
				{
					float y = blockPos.y + (blockScale.y * 0.5f) + 0.0002f;
					if (y > targetY)
					{
						targetY = y;
					}
				}
			}
		}
	}

	if( targetY == FLT_MAX)
	{
		return Vector3::Zero();
	}

	return Vector3(pos.x, targetY, pos.z);

}

bool CreateVerticalTarget(const Vector3& pos, const int lockonFrame, const float radius)
{
	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_target[i].isActive == false)
		{
			g_target[i].position = CheckTargetOnField(pos);
			if (g_target[i].position == Vector3::Zero())
			{
				return false;
			}
			g_target[i].lifeTime->SetInterval(lockonFrame);
			g_target[i].lifeTime->Reset();
			g_target[i].isActive = true;
			g_target[i].radius = radius;
			return true;
		}
	}

	return false;
}

