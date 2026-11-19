#include "camera.h"
#include "vector3.h"
#include "input.h"
#include "utility.h"
#include "shooter.h"
#include "player.h"
static constexpr float GROUND_Y = 10.0f;

static constexpr float MAX_PITCH = 45.0f * (3.141592f / 180.0f);
static constexpr float MIN_PITCH = 10.0f * (3.141592f / 180.0f);

struct Camera
{
	Vector3 pos;
	float pitch;
	float yaw;
	float distance;

	FrameTime* shakeTimer = nullptr;
	float shakePower = 0.0f;
	float shakeStartPower = 0.0f;
	Vector3 shakeOffset = Vector3::Zero();

	bool isOnShooter = false;
	FrameTime* onShooterTimer = nullptr;

};

static Camera g_camera;
static Vector3 g_targetPos;
static float g_sensitivity = 0.002f;


static constexpr Vector3 ON_SHOOTER_CAMERA_POSITION[4]
	=	{
			SHOOTER_POSITION[0] + Vector3(-5.0f, 1.0f, -5.0f),
			SHOOTER_POSITION[1] + Vector3( 5.0f, 1.0f, -5.0f),
			SHOOTER_POSITION[2] + Vector3( 5.0f, 1.0f,  5.0f),
			SHOOTER_POSITION[3] + Vector3(-5.0f, 1.0f,  5.0f)			
		};
static float g_targetYaw = 0.0f;
static float g_targetPitch = 0.0f;

void CameraInitilize()
{
	float initialPicth = DegreeToRadian(15.0f);
	g_camera.pos = { 0.0f,5.0f,0.0f };
	g_camera.pitch = initialPicth;
	g_camera.yaw = 0.0f;
	g_camera.distance = 8.0f;
	g_camera.shakeTimer = new FrameTime(0);
	g_camera.onShooterTimer = new FrameTime(45);
	g_camera.shakePower = 0.0f;
	g_camera.shakeOffset = Vector3::Zero();
	g_camera.isOnShooter = false;

	g_targetPos = { 0.0f,0.0f,0.0f };

	g_targetPitch = initialPicth;
	g_targetYaw = 0.0f;
}

void CameraFinalize()
{
	delete g_camera.shakeTimer;
	delete g_camera.onShooterTimer;
}

void CameraUpdate()
{
	// ターゲット位置の更新
	Vector3 diff = GetPlayerPosision() - g_targetPos;
	g_targetPos = diff * 0.5f + g_targetPos;

	if (GetPlayerState() == PlayerState::CHARGE)
	{
		int shooterIndex = GetOnShooterIndex();

		if(g_camera.isOnShooter == false)
		{
			g_camera.isOnShooter = true;
			g_camera.onShooterTimer->Reset();
		}

		float t = g_camera.onShooterTimer->GetElapsed() / static_cast<float>(g_camera.onShooterTimer->GetInterval());
		t = EaseOutQuint(t);
		Vector3 pos = Vector3::Lerp(g_camera.pos, ON_SHOOTER_CAMERA_POSITION[shooterIndex], t);
		g_camera.pos = pos;


		// 角度は使わないが更新しておく
		Vector3 dir = (g_targetPos - g_camera.pos).Normalized();

		g_targetPitch = asinf(dir.y);
		g_targetYaw = atan2f(dir.x, -dir.z) + XM_PI;

		g_camera.pitch = g_targetPitch;
		g_camera.yaw = g_targetYaw;
	}
	else
	{
		// 通常カメラ処理
		g_camera.isOnShooter = false;

		g_targetYaw -= Input::Mouse_Pos().x * g_sensitivity;
		g_targetPitch += Input::Mouse_Pos().y * g_sensitivity;
		g_targetPitch = Clamp(MIN_PITCH, MAX_PITCH, g_targetPitch);

		g_camera.yaw = Lerp(g_camera.yaw, g_targetYaw, 0.5f);
		g_camera.pitch = Lerp(g_camera.pitch, g_targetPitch, 0.5f);


		Vector3 offset;
		offset.x = cosf(g_camera.pitch) * sinf(g_camera.yaw) * g_camera.distance;
		offset.y = sinf(g_camera.pitch) * g_camera.distance;
		offset.z = cosf(g_camera.pitch) * cosf(g_camera.yaw) * -g_camera.distance;

		g_camera.pos = g_targetPos + offset;
	}



	if(g_camera.shakeTimer->IsFinished())
	{
		g_camera.shakePower = 0.0f;
		g_camera.shakeTimer->SetInterval(0);
	}
	else
	{
		float t = g_camera.shakeTimer->GetElapsed() / static_cast<float>(g_camera.shakeTimer->GetInterval());
		g_camera.shakePower = Lerp(g_camera.shakeStartPower, 0.0f, t);
		static Vector3 noise = Vector3::Zero();

		Vector3 targetNoise(
			RandomRange(-1.0f, 1.0f),
			RandomRange(-1.0f, 1.0f),
			0.0f
		);

		// 前フレームと補間
		noise.x = Lerp(noise.x, targetNoise.x, 0.2f);
		noise.y = Lerp(noise.y, targetNoise.y, 0.2f);

		Vector3 right = Vector3::Cross(GetCameraForward(), Vector3::Up()).Normalized();

		g_camera.shakeOffset =
			(right * noise.x + Vector3::Up() * noise.y) * g_camera.shakePower;
	}


}

void CameraDraw()
{
}

const XMMATRIX GetCameraViewMatrix()
{
	XMFLOAT3 up = { 0.0f,1.0f,0.0f };
	XMFLOAT3 pos = (g_camera.pos + g_camera.shakeOffset).ToXMFLOAT3();
	XMFLOAT3 target = g_targetPos.ToXMFLOAT3();
	return XMMatrixLookAtLH(XMLoadFloat3(&pos),
							XMLoadFloat3(&target),
							XMLoadFloat3(&up)
							);
}

const XMMATRIX GetBillboardMatrix()
{
	XMMATRIX ret = GetCameraViewMatrix();
	ret = XMMatrixInverse(nullptr, ret);
	// 位置成分を0にする
	ret.r[3].m128_f32[0] = 0.0f;
	ret.r[3].m128_f32[1] = 0.0f;
	ret.r[3].m128_f32[2] = 0.0f;
	return ret;
}

const Vector3& GetCameraPos()
{
	return g_camera.pos;
}

Vector3 GetCameraForward()
{	
	return Vector3(GetPlayerPosision() - g_camera.pos).Normalized();
}

void CameraShake(float power, int time)
{
	g_camera.shakePower = power;
	g_camera.shakeStartPower = power;
	g_camera.shakeTimer->SetInterval(time);
	g_camera.shakeTimer->Reset();
}

void SetSensitivity(float sensitivity)
{
	g_sensitivity = sensitivity;
}

float GetSensitivity()
{
	return g_sensitivity;
}


