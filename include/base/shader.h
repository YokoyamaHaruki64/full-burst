/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "light.h"
struct Rimlight
{
	DirectX::XMFLOAT3 cameraPos = { 0.0f,0.0f,0.0f };
	float rimPower = 0.0f;
	DirectX::XMFLOAT3 color{0.0f,0.0f,0.0f };
private:
		float pad0; // パディング
};

struct Matrix
{
    XMMATRIX matrix;
	XMMATRIX world;
};

struct InstanceVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texcoord;
};

struct InstanceData
{
	DirectX::XMFLOAT3 position;
	float scale;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT4 color;
};

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix_2D(const XMMATRIX& matrix);
void Shader_SetMatrix_3D(const Matrix& matrix, bool isInstance = false);
void Shader_SetMatrix_Shadow(const Matrix& matrix);
void InstanceShader_SetMatrix(const XMMATRIX& matrix);

void Shader_SetLight(const Light& light, bool isInstance = false);
void Shader_SetRimlight(const Rimlight& rim);
void Shader_SetTime(float time);
void Shader_Begin_2D();
void Shader_Begin_3D(bool isInstance = false);
void InstanceShader_Begin();
void Shader_Begin_Shadow();
void Shader_End_Shadow();


#endif // SHADER_H
