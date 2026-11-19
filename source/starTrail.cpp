#include "starTrail.h"
#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

struct StarTrail
{
	bool active;

	Vector3 pos;
	Vector3 vel;
	Vector3 rot;
	Vector3 angVel;
	Color color;
	float scale;
	float startScale;

	FrameTime* life;
};

static constexpr int MAX_STAR_TRAIL = 1024;

static constexpr Vector3 OFFSET_POS_RANGE = Vector3(0.25f,0.25f,0.25f);
static constexpr Vector3 VELOCITY_RANGE = Vector3(1.0f,1.0f,1.0f);
static constexpr Vector3 ANGULAR_VELOCITY_RANGE = Vector3(1.0f, 1.0f, 1.0f);
static constexpr float SCALE_MIN = 0.15f;
static constexpr float SCALE_MAX = 0.3f;
static constexpr int SATURATION = 30;
static constexpr int BRIGHTNESS = 100;

static StarTrail g_starTrails[MAX_STAR_TRAIL];

static int g_starTrailTexture;
static ID3D11Buffer* g_VertexBuffer;
static ID3D11Buffer* g_InstanceBuffer;
static float dt = 0.016f;

void StarTrailInitialize()
{
	for(int i = 0; i < MAX_STAR_TRAIL; i++)
	{
		g_starTrails[i].active = false;
		g_starTrails[i].life = new FrameTime(60);
	}

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(InstanceVertex) * MAX_STAR_TRAIL;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);

	bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(InstanceData) * MAX_STAR_TRAIL;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_InstanceBuffer);

	g_starTrailTexture = TextureLoad(L"asset\\texture\\star.png");

}

void StarTrailFinalize()
{
	for(int i = 0; i < MAX_STAR_TRAIL; i++)
	{
		if (g_starTrails[i].life)
		{
			delete g_starTrails[i].life;
			g_starTrails[i].life = nullptr;
		}
	}
}

void StarTrailUpdate()
{
	for(int i = 0; i < MAX_STAR_TRAIL; i++)
	{
		if(g_starTrails[i].active == false)
		{
			continue;
		}

		// 位置更新
		g_starTrails[i].pos += g_starTrails[i].vel * dt;
		// 回転更新
		g_starTrails[i].rot += g_starTrails[i].angVel * dt;

		float t = static_cast<float>(g_starTrails[i].life->GetElapsed()) / static_cast<float>(g_starTrails[i].life->GetInterval());
		float easedT = EaseInQuad(t);

		// スケール更新
		g_starTrails[i].scale = Lerp(g_starTrails[i].startScale, 0.0f, easedT);
		// 透明度更新
		g_starTrails[i].color.a = Lerp(1.0f, 0.0f, easedT);

		// ライフ更新
		if(g_starTrails[i].life->IsFinished())
		{
			g_starTrails[i].active = false;
		}
	}
}

void StarTrailDraw()
{
	// 深度バッファ有効化
	IsDepthEnable(true);
	SetBlendState(ADD);

	// インスタンシングで描画
	InstanceShader_Begin();

	// テクスチャ設定
	ID3D11ShaderResourceView* texture = GetTexture(g_starTrailTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	// ビュープロジェクション行列設定
	XMMATRIX viewProj = XMMatrixIdentity();
	viewProj *= GetCameraViewMatrix();
	viewProj *= GetProjectionMatrix();
	InstanceShader_SetMatrix(viewProj);

	// 頂点データ作成
	D3D11_MAPPED_SUBRESOURCE msrV = {};
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msrV);
	InstanceVertex* vertexData = (InstanceVertex*)msrV.pData;
	vertexData[0].pos = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertexData[1].pos = XMFLOAT3(0.5f, 0.5f, 0.0f);
	vertexData[2].pos = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertexData[3].pos = XMFLOAT3(0.5f, -0.5f, 0.0f);

	vertexData[0].texcoord = XMFLOAT2(0.0f, 0.0f);
	vertexData[1].texcoord = XMFLOAT2(1.0f, 0.0f);
	vertexData[2].texcoord = XMFLOAT2(0.0f, 1.0f);
	vertexData[3].texcoord = XMFLOAT2(1.0f, 1.0f);

	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);

	// インスタンスデータ作成
	D3D11_MAPPED_SUBRESOURCE msr = {};
	DirectXGetDeviceContext()->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	InstanceData* instanceData = (InstanceData*)msr.pData;

	int instanceCount = 0;
	for(int i = 0; i < MAX_STAR_TRAIL; i++)
	{
		if(g_starTrails[i].active == false)
		{
			continue;
		}

		// インスタンスデータ設定
		instanceData[instanceCount].position = g_starTrails[i].pos.ToXMFLOAT3();
		instanceData[instanceCount].scale = g_starTrails[i].scale;
		instanceData[instanceCount].rotation = g_starTrails[i].rot.ToXMFLOAT3();;
		instanceData[instanceCount].color = g_starTrails[i].color.ToXMFLOAT4();
		instanceCount++;
	}

	DirectXGetDeviceContext()->Unmap(g_InstanceBuffer, 0);

	// 描画
	UINT stride = sizeof(InstanceVertex);	// 1頂点あたりのサイズ
	UINT offset = 0;

	// 頂点バッファセット
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	stride = sizeof(InstanceData);			// 1インスタンスあたりのサイズ
	// インスタンスバッファセット
	DirectXGetDeviceContext()->IASetVertexBuffers(1, 1, &g_InstanceBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// インスタンス描画
	DirectXGetDeviceContext()->DrawInstanced(4, instanceCount, 0, 0);

	SetBlendState(ALPHABLEND);
}

void CreateStarTrail(const Vector3 pos)
{
	for(int i = 0; i < MAX_STAR_TRAIL; i++)
	{
		if(g_starTrails[i].active == false)
		{
			g_starTrails[i].active = true;

			Vector3 posOffset = RandomVector3(OFFSET_POS_RANGE * -1.0f, OFFSET_POS_RANGE);
			int hue = RandomRange(0, 360);

			g_starTrails[i].pos = pos + posOffset;
			g_starTrails[i].vel = RandomVector3(VELOCITY_RANGE * -1.0f, VELOCITY_RANGE);
			g_starTrails[i].rot = RandomVector3(Vector3::Zero(), Vector3::One() * 360.0f);
			g_starTrails[i].angVel = RandomVector3(ANGULAR_VELOCITY_RANGE * -1.0f, ANGULAR_VELOCITY_RANGE);
			g_starTrails[i].color = Color::HSVtoRGB(hue, SATURATION, BRIGHTNESS, 1.0f);
			g_starTrails[i].scale = RandomRange(SCALE_MIN, SCALE_MAX);
			g_starTrails[i].startScale = g_starTrails[i].scale;

			g_starTrails[i].life->SetInterval(60);
			g_starTrails[i].life->Reset();
			break;
		}
	}
}