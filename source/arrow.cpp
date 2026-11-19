#include "arrow.h"
#include "utility.h"
#include "directx.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "player.h"
#include "shooter.h"
#include "boss.h"

static MODEL* g_model = nullptr;

static constexpr Vector3 ARROW_POSITION[4] = {
	SHOOTER_POSITION[0] + Vector3(0.0f, 2.5f, 0.0f),
	SHOOTER_POSITION[1] + Vector3(0.0f, 2.5f, 0.0f),
	SHOOTER_POSITION[2] + Vector3(0.0f, 2.5f, 0.0f),
	SHOOTER_POSITION[3] + Vector3(0.0f, 2.5f, 0.0f),
};

static constexpr Vector3 PHASE4_OFFSETS[4] = {
	Vector3(0.0f, 2.0f, 0.0f),
	Vector3(0.0f, 2.0f, 0.0f),
	Vector3(0.0f, 2.0f, 0.0f),
	Vector3(0.0f, 2.0f, 0.0f),
};

static float g_positionYoffset = 0.0f;
static float g_rotationY = 0.0f;

void ArrowInitialize()
{
	g_model = ModelLoad("asset\\model\\arrow.fbx");
	g_positionYoffset = 0.0f;
	g_rotationY = 0.0f;
}

void ArrowFinalize()
{
	ModelRelease(g_model);
}

void ArrowUpdate()
{
	PlayerState state = GetPlayerState();

	if(state != PlayerState::CHARGE &&
	   state != PlayerState::SHOT)
	{
		g_positionYoffset = sinf(FrameTime::totalFrame * 0.06f) * 0.3f;
		g_rotationY += DegreeToRadian(3.0f);
		if (g_rotationY > XM_2PI)
		{
			g_rotationY -= XM_2PI;
		}
	}

}

void ArrowShadowDraw()
{
	PlayerState playerState = GetPlayerState();
	BossState bossState = GetBossState();

	if (playerState == PlayerState::CHARGE ||
		playerState == PlayerState::SHOT)
	{
		return;
	}


	for (int i = 0; i < 4; i++)
	{
		if (IsShooterActive(i) == false && bossState != BossState::PHASE4)
		{
			continue;
		}

		//頂点シェーダーに変換行列を設定
		Matrix mtx;
		float scale = bossState == BossState::PHASE4 ? 0.5f : 1.0f;
		Vector3 pos = bossState == BossState::PHASE4 ?
			RETURN_POSITIONS[i] + PHASE4_OFFSETS[i] :
			ARROW_POSITION[i];
		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixScaling(scale, scale, scale);
		mtx.world *= XMMatrixRotationY(g_rotationY);
		mtx.world *= XMMatrixTranslation(
			pos.x,
			pos.y + g_positionYoffset,
			pos.z);

		mtx.matrix = GetLightViewProj();
		Shader_SetMatrix_Shadow(mtx);
		ModelDrawShadow(g_model);
	}

}

void ArrowDraw()
{

	PlayerState playerState = GetPlayerState();
	BossState bossState = GetBossState();
	if (playerState == PlayerState::CHARGE ||
		playerState == PlayerState::SHOT)
	{
		return;
	}
	IsDepthEnable(true);

	Shader_Begin_3D();
	for (int i = 0; i < 4; i++)
	{
		if (IsShooterActive(i) == false && bossState != BossState::PHASE4)
		{
			continue;
		}

		Matrix mtx;
		float scale = bossState == BossState::PHASE4 ? 0.5f : 1.0f;
		Vector3 pos = bossState == BossState::PHASE4 ?
			RETURN_POSITIONS[i] + PHASE4_OFFSETS[i] :
			ARROW_POSITION[i];
		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixScaling(scale, scale, scale);
		mtx.world *= XMMatrixRotationY(g_rotationY);
		mtx.world *= XMMatrixTranslation(
			pos.x,
			pos.y + g_positionYoffset,
			pos.z);

		mtx.matrix = mtx.world;

		mtx.matrix *= GetCameraViewMatrix();
		mtx.matrix *= GetProjectionMatrix();

		Shader_SetMatrix_3D(mtx);
		Rimlight rim;
		rim.color = { 1.0f, 1.0f, 1.0f };
		rim.cameraPos = GetCameraPos().ToXMFLOAT3();
		rim.rimPower = 3.0f;
		Shader_SetRimlight(rim);

		ModelDraw(g_model);
	}
	IsDepthEnable(false);
}

