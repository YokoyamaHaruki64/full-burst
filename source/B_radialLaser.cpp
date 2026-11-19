#include "B_radialLaser.h"
#include "utility.h"
#include "directx.h"
#include "model.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "sprite.h"

#include "field.h"
#include "player.h"
#include "fieldDecal.h"

#include <chrono>

static constexpr float COLLIDER_LENGTH = 100.0f;
static constexpr float END_PICHE = 120.0f;

static RadialLaser g_laser[MAX_RADIAL_LASER];

static MODEL* g_laserModel;
static ID3D11Buffer* g_instanceBuffer;
static float dt = 0.016f;
static float g_damage = 1.0f;

Vector3 HitGroundPosition(const RadialLaser& laser, Block& outBlock);



void RadialLaserInitialize()
{
	g_laserModel = ModelLoad("asset\\model\\laser03.fbx");

	for (int i = 0; i < MAX_RADIAL_LASER; i++)
	{
		g_laser[i].isActive = false;

		g_laser[i].collider.radius = RADIALLASER_RADIUS;
		g_laser[i].collider.start = Vector3::Zero();
		g_laser[i].collider.end = Vector3::Zero();
	}
	g_damage = 1.0f;

	// インスタンスバッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(XMFLOAT4X4) * MAX_RADIAL_LASER;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_instanceBuffer);

}

void RadialLaserFinalize()
{
	SAFE_RELEASE(g_instanceBuffer);
}

void RadialLaserUpdate()
{
	for (int i = 0; i < MAX_RADIAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			continue;
		}

		// 回転更新
		float t = InverseLerp(g_laser[i].startFrame, g_laser[i].startFrame + g_laser[i].maxFrame, FrameTime::sceneFrame);
		t = EaseInCubic(t);
		g_laser[i].rotation.x = Lerp(g_laser[i].startPitch, END_PICHE, t);

		// ピッチから「ローカルY方向」をワールドに変換したベクトルを求める
		float pitch = XMConvertToRadians(g_laser[i].rotation.x - 90.0f);
		float yaw   = XMConvertToRadians(g_laser[i].rotation.y);

		// ピッチを乗せた3D方向（レーザーの伸びる方向：ローカル＋Y軸相当）
		Vector3 dir3D;
		dir3D.x = g_laser[i].directionXZ.x * cosf(pitch);
		dir3D.y = sinf(pitch);
		dir3D.z = g_laser[i].directionXZ.z * cosf(pitch);
		dir3D   = dir3D.Normalized();


		// 先端を startPosition に固定
		// 地面との当たり判定用に長くとる
		// center = start + dir3D * (COLLIDER_LENGTH * 0.5f)
		g_laser[i].centerPosition = g_laser[i].startPosition + dir3D * (COLLIDER_LENGTH * 0.5f);

		// コライダーも追従
		g_laser[i].length = COLLIDER_LENGTH;
		g_laser[i].collider.end = g_laser[i].centerPosition + dir3D * (COLLIDER_LENGTH * 0.5f);

		// 地面との当たり判定
		Block hitBlock;
		Vector3 hitPos = HitGroundPosition(g_laser[i], hitBlock);
		if(hitPos != Vector3::Zero())
		{

			g_laser[i].length = (hitPos - g_laser[i].startPosition).Length() + 0.5f; // 少し伸ばす
			// 中心点を更新
			g_laser[i].centerPosition = g_laser[i].startPosition + dir3D * (g_laser[i].length * 0.5f);
			// コライダーの終点も更新
			g_laser[i].collider.end = g_laser[i].centerPosition + dir3D * (g_laser[i].length * 0.5f);
		
			// デカール作成
			Vector3 decalPos = hitPos;
			decalPos.y = hitBlock.position.y + hitBlock.scale.y * 0.5f + 0.0001f; // 少し浮かせる
			SetFieldDecalWidth(g_laser[i].radius);
			if(g_laser[i].decalIndex == -1)
			{
				g_laser[i].decalIndex = CreateFieldDecal(decalPos);
			}
			else
			{
				AddFieldDecalPosition(g_laser[i].decalIndex, decalPos);
			}
		}

		// プレイヤーとの当たり判定
		if (GetPlayerState() == PlayerState::MOVE)
		{

			Vector3 playerHitPos = CapsuleVSSphere(g_laser[i].collider, GetPlayerCollider());
			if (playerHitPos != Vector3::Zero())
			{
				playerHitPos.y = 0.0f; // 水平のみの方向にする
				Vector3 dir = playerHitPos.Normalized();
				PlayerDamage(g_damage, dir, 30);
			}
		}

		if (t >= 1.0f)
		{
			g_laser[i].isActive = false;
		}
	}
}

void RadialLaserDraw()
{
	IsDepthEnable(true);

	const bool isInstance = true;
	Shader_Begin_3D(isInstance);

	// ライト設定
	Shader_SetLight(GetLight(), isInstance);

	// ビュープロジェクション行列のみ設定して渡す
	Matrix viewProj;
	viewProj.matrix = XMMatrixIdentity();
	viewProj.matrix *= GetCameraViewMatrix();
	viewProj.matrix *= GetProjectionMatrix();
	Shader_SetMatrix_3D(viewProj, isInstance);

	// リムライト設定
	Rimlight rim;
	Shader_SetRimlight(rim);

	D3D11_MAPPED_SUBRESOURCE msr = {};
	DirectXGetDeviceContext()->Map(g_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	XMFLOAT4X4* instanceData = (XMFLOAT4X4*)msr.pData;

	int instanceCount = 0;
	for (int i = 0; i < MAX_RADIAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			continue;
		}

		//頂点シェーダーに変換行列を設定
		XMFLOAT4X4 data;
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90.0f), 0.0f, 0.0f);
		world *= XMMatrixScaling(g_laser[i].radius, g_laser[i].length * 0.5f, g_laser[i].radius);
		world *= XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(g_laser[i].rotation.x),
			XMConvertToRadians(g_laser[i].rotation.y),
			XMConvertToRadians(g_laser[i].rotation.z));
		world *= XMMatrixTranslation(g_laser[i].centerPosition.x, g_laser[i].centerPosition.y, g_laser[i].centerPosition.z);
		XMStoreFloat4x4(&data, world);
		instanceData[instanceCount] = data;
		instanceCount++;
	}

	DirectXGetDeviceContext()->Unmap(g_instanceBuffer, 0);

	ModelDrawInstanced(g_laserModel, g_instanceBuffer, instanceCount);

	//Shader_Begin_3D();
	//Rimlight rim;
	//Shader_SetRimlight(rim);

	//for (int i = 0; i < MAX_RADIAL_LASER; i++)
	//{
	//	if (g_laser[i].isActive == false)
	//	{
	//		continue;
	//	}

	//	Matrix mtx;
	//	mtx.world = XMMatrixIdentity();
	//	mtx.world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90.0f), 0.0f, 0.0f);
	//	mtx.world *= XMMatrixScaling(g_laser[i].radius, g_laser[i].length * 0.5f, g_laser[i].radius);
	//	mtx.world *= XMMatrixRotationRollPitchYaw(
	//		XMConvertToRadians(g_laser[i].rotation.x),
	//		XMConvertToRadians(g_laser[i].rotation.y),
	//		XMConvertToRadians(g_laser[i].rotation.z));

	//	mtx.world *= XMMatrixTranslation(g_laser[i].centerPosition.x, g_laser[i].centerPosition.y, g_laser[i].centerPosition.z);

	//	mtx.matrix = mtx.world;

	//	mtx.matrix *= GetCameraViewMatrix();
	//	mtx.matrix *= GetProjectionMatrix();

	//	Shader_SetMatrix_3D(mtx);

	//	ModelDraw(g_laserModel);

	//}

}

void CreateRadialLaser(const Vector3& startPos, const Vector3 dir, const float radius, const int maxFrame)
{
	for (int i = 0; i < MAX_RADIAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			Vector3 nDir = dir.Normalized();

			float yaw = atan2f(nDir.z, nDir.x);
			float pitch = atan2f(
				nDir.y,
				sqrtf(nDir.x * nDir.x + nDir.z * nDir.z)
			);


			g_laser[i].isActive = true;
			g_laser[i].startPosition = startPos;
			g_laser[i].centerPosition = startPos + nDir * (COLLIDER_LENGTH * 0.5f);
			g_laser[i].directionXZ = Vector3(nDir.x,0.0f, nDir.z).Normalized();
			g_laser[i].rotation.x = XMConvertToDegrees(pitch) + 90.0f;
			g_laser[i].startPitch = g_laser[i].rotation.x;
			g_laser[i].rotation.y = -(XMConvertToDegrees(yaw) + 90.0f);
			g_laser[i].length = COLLIDER_LENGTH;
			g_laser[i].radius = radius;
			g_laser[i].collider.start = g_laser[i].startPosition;
			g_laser[i].collider.end = g_laser[i].centerPosition + nDir * (COLLIDER_LENGTH * 0.5f);
			g_laser[i].collider.radius = radius;
			g_laser[i].decalIndex = -1;
			g_laser[i].startFrame = FrameTime::sceneFrame;
			g_laser[i].maxFrame = maxFrame;
			break;
		}
	}
}

Vector3 HitGroundPosition(const RadialLaser& laser, Block& outBlock)
{
	std::vector<Block> block = GetBlocks();

	Vector3 min = Vector3::Zero();
	for (const auto& b : block)
	{
		// 地面ブロック以外は無視
		if (b.type != 1)
		{
			continue;
		}
		// 衝突判定(太さを持った線分とAABBの当たり判定)
		Vector3 hitPos = CapsuleVSAABB_HitPos(laser.collider, b.collider);
		if (hitPos != Vector3::Zero())
		{
			// 最も近い衝突点を採用
			if (min == Vector3::Zero())
			{
				min = hitPos;
				outBlock = b;
			}
			else
			{
				if ((hitPos - laser.startPosition).SqrLength() < (min - laser.startPosition).SqrLength())
				{
					min = hitPos;
					outBlock = b;
				}
			}
		}

	}
	//outBlock = b;
	return min;

	return Vector3::Zero();

}

void SetRadialLaserDamage(float damage)
{
	g_damage = damage;
}