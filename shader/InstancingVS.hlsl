struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    
    float3 iPos : INSTANCE_POS;
    float iScale : INSTANCE_SCALE;
    float3 iRot : INSTANCE_ROT;
    float4 iColor : INSTANCE_COLOR;
};


struct VS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0; // 頂点カラー
    float2 texcoord : TEXCOORD0;
};

// ビューとプロジェクションの行列
cbuffer CameraMatrix : register(b0)
{
    float4x4 mtx;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float sx, cx;
    float sy, cy;
    float sz, cz;

    sincos(input.iRot.x, sx, cx);
    sincos(input.iRot.y, sy, cy);
    sincos(input.iRot.z, sz, cz);
    
    // スケール → 回転(X, Y, Z) の順でインスタンス変換を適用
    float3 localPos = input.position * input.iScale;

    // X 回転
    float3 p;
    p.x = localPos.x;
    p.y = cx * localPos.y - sx * localPos.z;
    p.z = sx * localPos.y + cx * localPos.z;

    // Y 回転
    float3 p2;
    p2.x = cy * p.x + sy * p.z;
    p2.y = p.y;
    p2.z = -sy * p.x + cy * p.z;

    // Z 回転
    float3 p3;
    p3.x = cz * p2.x - sz * p2.y;
    p3.y = sz * p2.x + cz * p2.y;
    p3.z = p2.z;

    // ワールド座標（平行移動）
    float3 worldPos = p3 + input.iPos;
        
    output.color = input.iColor;
    output.texcoord = input.texcoord;
    // クリップ空間への変換
    output.position = mul(float4(worldPos, 1.0f), mtx);
    
    return output;
}