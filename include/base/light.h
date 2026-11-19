#pragma once

#include <DirectXMath.h>

struct Light
{
    // 定数バッファは 16 バイト境界でサイズが揃っている必要があるため
    // 明示的にパディングを加える
    // publicメンバ変数の後ろにprivateメンバ変数としてfloat型の配列を追加することでパディングを実現している

    int enable;
private:
    float pad0[3];
public:
    DirectX::XMFLOAT3 directionXZ;
private:
    float pad1;
public:
    DirectX::XMFLOAT3 color;
    float ambientIntensity;

};
void LightInitialize();

void SetLightDirection(const DirectX::XMFLOAT3& dir);
void SetLightColor(const DirectX::XMFLOAT3& color);
void SetLightAmbientIntensity(float intensity);
void EnableLight(bool enable);
const DirectX::XMMATRIX GetLightViewProj();

const Light& GetLight();

