
struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0; // 頂点カラー
    float2 texcoord : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = tex.Sample(samplerState, input.texcoord); // テクスチャ色
    return texColor * input.color; // テクスチャの色と頂点カラーを掛ける
}