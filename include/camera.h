#pragma once
#include "directx.h"
#include "vector3.h"

static constexpr float MIN_SENSITIVITY = 0.0001f;
static constexpr float MAX_SENSITIVITY = 0.01f;

void CameraInitilize();
void CameraFinalize();
void CameraUpdate();
void CameraDraw();
const XMMATRIX GetCameraViewMatrix();
const XMMATRIX GetBillboardMatrix();
inline const XMMATRIX GetProjectionMatrix()
{
	return XMMatrixPerspectiveFovLH(1.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT,
		1.0f, 1000.0f);
}
const Vector3& GetCameraPos();
Vector3 GetCameraForward();

void CameraShake(float power, int time);

void SetSensitivity(float sensitivity);
float GetSensitivity();
