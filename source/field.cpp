#include "field.h"
#include "directx.h"
#include "model.h"
#include "shader.h"
#include "camera.h"

std::vector<Block>& GetBlocks();
static MODEL* g_blockModel = nullptr;
static std::vector<Block> blocks;

static ID3D11Buffer* g_instanceBuffer = nullptr;

void FieldInitialize()
{
	g_blockModel = ModelLoad("asset\\model\\tile01.fbx");

	for (int i = 0; i < 4; i++)
	{
		Vector3 offset = TABLE_OFFSET[i];
		for (int x = 0; x < TABLE_SIZE; x++)
		{
			for (int z = 0; z < TABLE_SIZE; z++)
			{
				Vector3 pos = Vector3(
					-(TABLE_SIZE / 2) + x + 0.5f,
					0.0f,
					(TABLE_SIZE - 1) - z - 0.5f
				);
				pos += offset;

				Block b;
				b.position = pos.ToXMFLOAT3();
				b.scale = { 1.0f , 1.0f , 1.0f };
				b.type = 1;
				b.collider.center = Vector3::FromXMFLOAT3(b.position);
				b.collider.size = Vector3::FromXMFLOAT3(b.scale);
				blocks.push_back(b);
			}
		}
	}


	for(int x = 0;x < FIELD_SIZE; x++)
	{
		for(int z = 0;z < FIELD_SIZE; z++)
		{
			if(	FIELD_SIZE / 2 - HALL_SIZE / 2 <= x &&
				FIELD_SIZE / 2 + HALL_SIZE / 2 > x &&
				FIELD_SIZE / 2 - HALL_SIZE / 2 <= z &&
				FIELD_SIZE / 2 + HALL_SIZE / 2 > z)
			{
				continue;
			}


			Block b;
			b.position = XMFLOAT3(
				-(FIELD_SIZE / 2) + x + 0.5f,
				0.0f,
				(FIELD_SIZE - 1) - z - 0.5f
			);
			b.scale = { 1.0f , 1.0f , 1.0f };
			b.type = 1;
			b.collider.center = Vector3::FromXMFLOAT3(b.position);
			b.collider.size = Vector3::FromXMFLOAT3(b.scale);
			blocks.push_back(b);
		}
	}

	// インスタンスバッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(XMFLOAT4X4) * static_cast<UINT>(blocks.size() * 1.5f);	// 十分なサイズを確保
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_instanceBuffer);

}

void FieldFinalize()
{
	ModelRelease(g_blockModel);
	SAFE_RELEASE(g_instanceBuffer);
	blocks.clear();
}

void FieldUpdate()
{
}

void FieldShadowDraw()
{
	for (auto& b : blocks)
	{
		XMFLOAT3 scale{ 1.0f,1.0f,1.0f };
		XMFLOAT3 pos = b.position;

		//頂点シェーダーに変換行列を設定
		Matrix mtx;
		mtx.world = XMMatrixIdentity();

		mtx.world *= XMMatrixScaling(scale.x,
			scale.y,
			scale.z);
		mtx.world *= XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		mtx.world *= XMMatrixTranslation(pos.x, pos.y, pos.z);
		mtx.matrix = GetLightViewProj();
		Shader_SetMatrix_Shadow(mtx);
		ModelDrawShadow(g_blockModel);
	}
}
void FieldDraw()
{
	IsDepthEnable(true);
	const bool isInstance = true;
	Shader_Begin_3D(isInstance);

	// ライト設定
	Shader_SetLight(GetLight(),isInstance);
	
	// ビュープロジェクション行列のみ設定して渡す
	Matrix viewProj;
	viewProj.matrix = XMMatrixIdentity();
	viewProj.matrix *= GetCameraViewMatrix();
	viewProj.matrix *= GetProjectionMatrix();
	Shader_SetMatrix_3D(viewProj,isInstance);

	// リムライト設定
	Rimlight rim;
	Shader_SetRimlight(rim);

	D3D11_MAPPED_SUBRESOURCE msr = {};
	DirectXGetDeviceContext()->Map(g_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	XMFLOAT4X4* instanceData = (XMFLOAT4X4*)msr.pData;

	int instanceCount = 0;
	for (auto& b : blocks)
	{
		XMFLOAT3 scale{ 1.0f,1.0f,1.0f };
		XMFLOAT3 pos = b.position;

		//頂点シェーダーに変換行列を設定
		XMFLOAT4X4 data;
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixScaling(scale.x,scale.y,scale.z);
		world *= XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMStoreFloat4x4(&data, world);

		instanceData[instanceCount] = data;
		instanceCount++;
	}

	DirectXGetDeviceContext()->Unmap(g_instanceBuffer, 0);
	
	ModelDrawInstanced(g_blockModel, g_instanceBuffer, instanceCount);

	IsDepthEnable(false);
}


std::vector<Block>& GetBlocks()
{
	return blocks;
}

void CreateWall(Vector3 pos)
{
	Vector3 center = pos;
	//center.x -= 0.5f;
	//center.z -= 0.5f;

	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 1; x++)
		{
			for (int z = 0; z < 1; z++)
			{
				Block b;
				b.position = XMFLOAT3(
					center.x + x,
					center.y + y,
					center.z + z
				);
				b.scale = { 1.0f , 1.0f , 1.0f };
				b.type = 1;
				b.collider.center = Vector3::FromXMFLOAT3(b.position);
				b.collider.size = Vector3::FromXMFLOAT3(b.scale);
				blocks.push_back(b);
			}
		}
	}

}