
struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0; // 頂点カラー
    float2 texcoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
    bool isLighting : TEXCOORD3;
};

cbuffer RimBuffer : register(b0)
{
    float3 cameraPos;
    float rimPower;
    float3 rimColor;
    float pad; // パディング用
};

cbuffer LightViewProjBuffer : register(b1)
{
    float4x4 LightViewProj;
}

cbuffer TimeBuffer : register(b2)
{
    float time;
    float3 pad2;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

Texture2D shadowMap : register(t1);
SamplerComparisonState shadowSampler : register(s1);

float CalcShadow(float3 worldPos)
{
    float4 lightPos = mul(float4(worldPos, 1.0), LightViewProj);
    lightPos.xyz /= lightPos.w;

    float2 uv = lightPos.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;

    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
        return 1.0;

    float depth = saturate(lightPos.z);
    float2 texelSize = 1.0 / 4096.0;
    
    float shadowSample = 0.0;

    // PCF 3x3サンプリング
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 offset = float2(x, y) * texelSize;
            shadowSample += shadowMap.SampleCmpLevelZero(shadowSampler, uv + offset, depth - 0.0002);
        }
    }

    shadowSample /= 9.0;
    float shadowMin = 0.5;
    return lerp(shadowMin, 1.0, shadowSample);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 view = normalize(cameraPos - input.worldPos);
    
    float rim = pow(saturate(1 - dot(input.normal, view)), rimPower);

    float3 finalRimColor = rim * rimColor;
    
    // テクスチャのサンプリング
    //
    float2 uv = input.texcoord;
    uv += float2(sin(time * 3.0), cos(time * 3.0)); // 時間で動かす
    float4 texColor = tex.Sample(samplerState, uv); // テクスチャ色  
    
    float shadow = input.isLighting ? CalcShadow(input.worldPos) : 1.0;
    
    float3 color = texColor.rgb * input.color.rgb * shadow + finalRimColor;
    float alpha = texColor.a * input.color.a;
    return float4(color, alpha);

}
