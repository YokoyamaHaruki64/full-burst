cbuffer ShadowMatrixBuffer : register(b0)
{
    float4x4 WorldMatrix;
    float4x4 LightViewProj;
};

struct VS_INPUT
{
    float4 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 worldPos = mul(input.position, WorldMatrix);
    output.position = mul(worldPos, LightViewProj);
    return output;
}