#include "light.h"
using namespace DirectX;

static Light g_light;

void LightInitialize()
{
	g_light.enable = false;
	g_light.directionXZ = { 0.0f, -1.0f, 0.0f };
	g_light.color = { 1.0f, 1.0f, 1.0f };
	g_light.ambientIntensity = 0.4f;
}

void SetLightDirection(const DirectX::XMFLOAT3& dir)
{
	g_light.directionXZ = dir;
}
void SetLightColor(const DirectX::XMFLOAT3& color)
{
	g_light.color = color;
}
void SetLightAmbientIntensity(float intensity)
{
	g_light.ambientIntensity = intensity;
}
void EnableLight(bool enable)
{
	g_light.enable = enable;
}

const XMMATRIX GetLightViewProj()
{

	XMFLOAT3 up = { 0.0f,1.0f,0.0f };
	XMFLOAT3 pos = { -g_light.directionXZ.x * 50.0f, -g_light.directionXZ.y * 50.0f, -g_light.directionXZ.z * 50.0f };

	XMMATRIX view = XMMatrixLookAtLH(
		XMLoadFloat3(&pos),
		XMVectorZero(),
		XMLoadFloat3(&up));

	XMMATRIX proj = XMMatrixOrthographicLH(100.0f, 100.0f, 0.1f, 200.0f);

	XMMATRIX ret = XMMatrixIdentity();
	ret *= view;
	ret *= proj;
	return ret;

}

const Light& GetLight()
{
	return g_light;
}


