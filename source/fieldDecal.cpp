#include "fieldDecal.h"
#include <vector>
#include "directx.h"
#include "sprite.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"	

struct FieldDecal
{
	bool isActive = false;
	std::vector<Vector3> positions;

	FrameTime* lifeTime = nullptr;
};

static constexpr int MAX_FIELD_DECAL = 128;
static constexpr int MAX_VERTEX = 1024;

static FieldDecal g_fieldDecals[MAX_FIELD_DECAL];
static ID3D11Buffer* g_vertexBuffer;
static int g_decalTexture;
static float g_decalWidth = 0.3f;


void FieldDecalInitialize()
{
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * (MAX_VERTEX);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_vertexBuffer);

	g_decalTexture = TextureLoad(L"asset\\texture\\burnMark.png");
	g_decalWidth = 0.3f;

	for(int i = 0; i < MAX_FIELD_DECAL; i++)
	{
		g_fieldDecals[i].isActive = false;
		g_fieldDecals[i].positions.clear();
		g_fieldDecals[i].lifeTime = nullptr;
	}

}

void FieldDecalFinalize()
{
	SAFE_RELEASE(g_vertexBuffer);

	for(int i = 0; i < MAX_FIELD_DECAL; i++)
	{
		if (g_fieldDecals[i].lifeTime)
		{
			delete g_fieldDecals[i].lifeTime;
			g_fieldDecals[i].lifeTime = nullptr;
		}
	}

}

void FieldDecalUpdate()
{
	for(int i = 0; i < MAX_FIELD_DECAL; i++)
	{
		if(g_fieldDecals[i].isActive == false)
		{
			continue;
		}
		if (g_fieldDecals[i].lifeTime->IsFinished())
		{
			g_fieldDecals[i].isActive = false;
			g_fieldDecals[i].positions.clear();
			delete g_fieldDecals[i].lifeTime;
			g_fieldDecals[i].lifeTime = nullptr;
		}
	}

}

void FieldDecalDraw()
{
	IsDepthEnable(true);


	for(int i = 0; i < MAX_FIELD_DECAL; i++)
	{
		if(g_fieldDecals[i].isActive == false)
		{
			continue;
		}

		if (g_fieldDecals[i].positions.size() < static_cast<size_t>(2))
		{
			continue;
		}

		float time_t = (float)g_fieldDecals[i].lifeTime->GetElapsed()
                     / g_fieldDecals[i].lifeTime->GetInterval();
		time_t = EaseInExpo(time_t);

		// 頂点データ
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(
            g_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        Vertex* v = (Vertex*)msr.pData;

        // 古い方から新しい方への向き
        Vector3 baseDir =
            (g_fieldDecals[i].positions.back()
           - g_fieldDecals[i].positions.front());

        Vector3 right = Vector3::Cross(Vector3::Up(), baseDir).Normalized();

        const auto& pts = g_fieldDecals[i].positions;
        const int count = static_cast<int>(pts.size());

        // 距離に基づいて v 座標を進める
        float vAccum = 0.0f;
        // 1ユニットあたりのv進行量（テクスチャ繰り返し頻度）
        const float vPerUnit = 0.2f; // 調整用：小さいほどタイル状になる

        for (int j = 0; j < count; j++)
        {
			float halfWidth = g_decalWidth * 0.5f;

            float lifeRatio = (float)j / (float)count;
            lifeRatio = Lerp(1.0f, lifeRatio, 0.3f);
            float alpha = lifeRatio * (1.0f - time_t);

            // 位置
            v[j * 2 + 0].pos = (pts[j] + right * halfWidth).ToXMFLOAT3();
            v[j * 2 + 1].pos = (pts[j] - right * halfWidth).ToXMFLOAT3();

            // j>0 のとき前の点との距離を加算してvを進める
            if (j > 0)
            {
                float segLen = (pts[j] - pts[j - 1]).Length();
                vAccum += segLen * vPerUnit;
            }
            float vCoord = vAccum;

            // UV設定（U方向は幅、V方向は長さ）
            v[j * 2 + 0].texcoord = XMFLOAT2(1.0f, vCoord);
            v[j * 2 + 1].texcoord = XMFLOAT2(0.0f, vCoord);

            v[j * 2 + 0].color = XMFLOAT4(1, 1, 1, alpha);
            v[j * 2 + 1].color = XMFLOAT4(1, 1, 1, alpha);

            v[j * 2 + 0].normal = XMFLOAT3(0, 1, 0);
            v[j * 2 + 1].normal = XMFLOAT3(0, 1, 0);
        }

        Shader_Begin_3D();

        Rimlight rim;
        Shader_SetRimlight(rim);

        Matrix matrix;
        matrix.world = XMMatrixIdentity();
        matrix.matrix = matrix.world;
        matrix.matrix *= GetCameraViewMatrix();
        matrix.matrix *= GetProjectionMatrix();
        Shader_SetMatrix_3D(matrix);

        DirectXGetDeviceContext()->Unmap(g_vertexBuffer, 0);

        // テクスチャセット
		ID3D11ShaderResourceView* tex = GetTexture(g_decalTexture);
		DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &tex);

		// 頂点バッファセット
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(
            0, 1, &g_vertexBuffer, &stride, &offset);

		// プリミティブトポロジ設定
		DirectXGetDeviceContext()->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		DirectXGetDeviceContext()->Draw(count * 2, 0);

	}
}

int CreateFieldDecal(const Vector3& pos)
{
	for(int i = 0; i < MAX_FIELD_DECAL; i++)
	{
		if(g_fieldDecals[i].isActive == false)
		{
			g_fieldDecals[i].isActive = true;
			g_fieldDecals[i].positions.clear();
			g_fieldDecals[i].positions.push_back(pos);
			g_fieldDecals[i].lifeTime = new FrameTime(300);
			return i;
		}
	}

	return -1;
}

void AddFieldDecalPosition(int index, const Vector3& pos)
{
	if (index < 0 || index >= MAX_FIELD_DECAL)
	{
		return;
	}
	if (g_fieldDecals[index].isActive == false)
	{
		return;
	}
	g_fieldDecals[index].positions.push_back(pos);
}

void SetFieldDecalWidth(float width)
{
	g_decalWidth = width;
}