#include "shooter.h"
#include "utility.h"
#include "directx.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "player.h"

struct Shooter
{
	bool isActive = true;
	Vector3 position{ Vector3::Zero() };
	Vector3 rotation{ Vector3::Zero() };
	Vector3 scale	{ Vector3::One() };
};

static constexpr float SHOOTER_ROTATE_ANGLE[4] 
= { -45.0f, -135.0f, 135.0f, 45.0f };

int IsOnShooter();


static MODEL* g_model = nullptr;
static Shooter g_shooter[4];

void ShooterInitialize()
{
	g_model = ModelLoad("asset\\model\\shooter.fbx");

	for (int i = 0; i < 4; i++)
	{
		g_shooter[i].isActive = true;
		g_shooter[i].position = SHOOTER_POSITION[i];
		g_shooter[i].rotation = Vector3(
			0.0f,
			DegreeToRadian(SHOOTER_ROTATE_ANGLE[i]),
			0.0f
		);
		g_shooter[i].scale = Vector3(1.0f, 1.0f, 1.0f);
	}

}

void ShooterFinalize()
{
	ModelRelease(g_model);
}

void ShooterUpdate()
{
	PlayerState state = GetPlayerState();
	int onShooter = IsOnShooter();
	if( onShooter >= 0 &&
		IsPlayerGround() &&
		!(state == PlayerState::CHARGE ||
		 state == PlayerState::SHOT))
	{
		SetPlayerState(PlayerState::CHARGE);
		SetShooter(-g_shooter[onShooter].rotation.y,onShooter);
		SetPlayerPosision(g_shooter[onShooter].position + Vector2(0.0f, 0.75f));
	}
}

void ShooterShadowDraw()
{
	for (int i = 0; i < 4; i++)
	{
		if (g_shooter[i].isActive == false)
		{
			continue;
		}

		//頂点シェーダーに変換行列を設定
		Matrix mtx;
		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixScaling(g_shooter[i].scale.x, g_shooter[i].scale.y, g_shooter[i].scale.z);
		XMVECTOR q = XMQuaternionRotationRollPitchYaw(
			g_shooter[i].rotation.x,
			g_shooter[i].rotation.y,
			g_shooter[i].rotation.z
		);
		mtx.world *= XMMatrixRotationQuaternion(q);
		mtx.world *= XMMatrixTranslation(g_shooter[i].position.x,
			g_shooter[i].position.y,
			g_shooter[i].position.z);
		mtx.matrix = GetLightViewProj();
		Shader_SetMatrix_Shadow(mtx);
		ModelDrawShadow(g_model);
	}

}

void ShooterDraw()
{
	IsDepthEnable(true);

	Shader_Begin_3D();
	for (int i = 0; i < 4; i++)
	{
		if(g_shooter[i].isActive == false)
		{
			continue;
		}
		//頂点シェーダーに変換行列を設定
		Matrix mtx;

		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixScaling(g_shooter[i].scale.x, g_shooter[i].scale.y, g_shooter[i].scale.z);

		XMVECTOR q = XMQuaternionRotationRollPitchYaw(
			g_shooter[i].rotation.x,
			g_shooter[i].rotation.y,
			g_shooter[i].rotation.z
		);

		mtx.world *= XMMatrixRotationQuaternion(q);

		mtx.world *= XMMatrixTranslation(g_shooter[i].position.x,
			g_shooter[i].position.y,
			g_shooter[i].position.z);

		mtx.matrix = mtx.world;

		mtx.matrix *= GetCameraViewMatrix();
		mtx.matrix *= GetProjectionMatrix();

		Shader_SetMatrix_3D(mtx);
		Rimlight rim;
		Shader_SetRimlight(rim);

		ModelDraw(g_model);
	}
	IsDepthEnable(false);
}

int IsOnShooter()
{
	for(int i = 0; i < 4; i++)
	{
		if(g_shooter[i].isActive == false)
		{
			continue;
		}

		Vector3 playerPos = GetPlayerPosision();
		Vector3 shooterPos = g_shooter[i].position;

		// プレイヤーの位置をシューターのローカル座標系に変換
		Vector3 localPos = playerPos - shooterPos;

		// Y軸周りの回転の逆変換を適用
		float cosY = cosf(-g_shooter[i].rotation.y);
		float sinY = sinf(-g_shooter[i].rotation.y);

		// 回転行列の逆行列を使って座標を変換
		float rotatedX = localPos.x * cosY - localPos.z * sinY;
		float rotatedZ = localPos.x * sinY + localPos.z * cosY;

		// 回転後のローカル座標で判定
		float halfSizeX = 0.5f;
		float halfSizeZ = 0.5f;

		if (rotatedX > -halfSizeX && rotatedX < halfSizeX &&
			rotatedZ > -halfSizeZ && rotatedZ < halfSizeZ)
		{
			return i;
		}
	}

	return -1;
}

void SetAllShooterActive(bool active)
{
	for(int i = 0; i < 4; i++)
	{
		g_shooter[i].isActive = active;
	}
}

void SetShooterActive(int index, bool active)
{
	if(index < 0 || index >= 4)
	{
		return;
	}

	g_shooter[index].isActive = active;
}

bool IsShooterActive(int index)
{
	return g_shooter[index].isActive;
}
