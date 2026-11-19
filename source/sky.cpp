#include "sky.h"
#include "directx.h"
#include "model.h"
#include "camera.h"
#include "shader.h"

static MODEL* g_model[SKY_MODEL_NUM]{};
static float g_rot = 0.0f;
static SkyModel currentSky = SKY_MODEL_01;
void SkyInitialize()
{
	g_model[0] = ModelLoad("asset\\model\\sky01.fbx");
	g_model[1] = ModelLoad("asset\\model\\sky02.fbx");
	g_model[2] = ModelLoad("asset\\model\\sky03.fbx");
	g_model[3] = ModelLoad("asset\\model\\sky04.fbx");

}

void SkyFinalize()
{
	for(int i = 0; i < SKY_MODEL_NUM; i++)
	{
		ModelRelease(g_model[i]);
	}
}

void SkyUpdate()
{
}

void SkyDraw()
{
	Shader_Begin_3D();

	g_rot += 0.001f;
	Matrix mtx;
	mtx.world = XMMatrixIdentity();
	mtx.world *= XMMatrixScaling(1000.0f, 1000.0f, 1000.0f);
	mtx.world *= XMMatrixRotationY(g_rot);
	mtx.world *= XMMatrixTranslation(GetCameraPos().x, GetCameraPos().y, GetCameraPos().z);

	mtx.matrix = mtx.world;

	mtx.matrix *= GetCameraViewMatrix();
	mtx.matrix *= GetProjectionMatrix();

	Rimlight rim;
	Shader_SetRimlight(rim);
	Shader_SetMatrix_3D(mtx);

	ModelDraw(g_model[currentSky]);
}

void ChangeSky(SkyModel skyNum)
{
	currentSky = skyNum;
}