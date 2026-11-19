#include "B_verticalTarget.h"
#include "B_verticalLaser.h"
#include <vector>
#include "utility.h"
#include "directx.h"
#include "model.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "sprite.h"
#include "sound.h"

#include "field.h"

static constexpr float LASER_SPEED = -180.0f;
static constexpr float RADIALLASER_RADIUS = 0.5f;
static constexpr float LASER_HEIGHT = 100.0f;

static VerticalLaser g_laser[MAX_VERTICAL_LASER];

static MODEL* g_laserModel;
static float dt = 0.016f;

static int g_laserSound;
static int g_targetLaserSound;

void VerticalLaserInitialize()
{
	g_laserModel = ModelLoad("asset\\model\\laser02.fbx");

	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		g_laser[i].isActive = false;

		g_laser[i].collider.radius = RADIALLASER_RADIUS;
		g_laser[i].collider.height = LASER_HEIGHT;
	}

	g_laserSound = LoadSound("asset\\sound\\laser.wav");
	g_targetLaserSound = LoadSound("asset\\sound\\targetLaser.wav");

	SetVolume(g_targetLaserSound, 0.5f);
}

void VerticalLaserFinalize()
{
	ModelRelease(g_laserModel);
}

void VerticalLaserUpdate()
{
	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			continue;
		}
		g_laser[i].position.y += LASER_SPEED * dt;
		g_laser[i].collider.center = g_laser[i].position;

		if(g_laser[i].position.y < -LASER_HEIGHT * 5.0f)
		{
			g_laser[i].isActive = false;
		}
	}
}

void VerticalLaserDraw()
{
	IsDepthEnable(true);

	Shader_Begin_3D();

	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			continue;
		}

		Matrix mtx;
		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90.0f), 0.0f, 0.0f);
		mtx.world *= XMMatrixScaling(g_laser[i].radius,LASER_HEIGHT * 0.5f, g_laser[i].radius);

		XMMATRIX billboard = GetBillboardMatrix();
		// Y軸固定ビルボード
		billboard.r[1] = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		mtx.world *= billboard;

		mtx.world *= XMMatrixTranslation(g_laser[i].position.x, g_laser[i].position.y, g_laser[i].position.z);

		mtx.matrix = mtx.world;

		mtx.matrix *= GetCameraViewMatrix();
		mtx.matrix *= GetProjectionMatrix();

		Shader_SetMatrix_3D(mtx);
		Rimlight rim;
		Shader_SetRimlight(rim);

		ModelDraw(g_laserModel);

	}
	IsDepthEnable(false);

}

void CreateVerticalLaser(const Vector3& pos, const float radius)
{
	for (int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if (g_laser[i].isActive == false)
		{
			g_laser[i].position = pos;
			g_laser[i].position.y += LASER_HEIGHT * 0.5f;
			g_laser[i].radius = radius;
			g_laser[i].isActive = true;
			g_laser[i].collider.center = pos;
			g_laser[i].collider.radius = radius;
			if(radius < 0.3f)
				PlaySound(g_laserSound,0);
			else
				PlaySound(g_targetLaserSound, 0);
			return;
		}
	}
}

VerticalLaser* GetVerticalLasers()
{
	return g_laser;
}
