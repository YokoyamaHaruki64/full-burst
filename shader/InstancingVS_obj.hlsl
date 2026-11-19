struct VS_INPUT
{
    float4 position : POSITION0;
    float4 color    : COLOR0;
    float2 texcoord : TEXCOORD0;
    float3 normal   : NORMAL0;
    
    float4 iWorld0 : INSTANCE_WORLD0; // インスタンス用ワールド行列
    float4 iWorld1 : INSTANCE_WORLD1; // インスタンス用ワールド行列
    float4 iWorld2 : INSTANCE_WORLD2; // インスタンス用ワールド行列
    float4 iWorld3 : INSTANCE_WORLD3; // インスタンス用ワールド行列
};


struct VS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0; // 頂点カラー
    float2 texcoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
    bool isLighting : TEXCOORD3;
};

//定数バッファ
cbuffer CameraMatrix : register(b0)
{
    float4x4 ViewProj; 
};

// ライティング用定数バッファ
cbuffer LightBuffer : register(b1)
{
    bool isLighting; // ライティングの有効/無効
    float3 pad0; // パディング用
    
    float3 lightDir; // 光の方向
    float pad1;
    
    float3 lightColor; // 光の色
    float ambientIntensity; // 環境光の強さ
    
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    row_major float4x4 worldMatrix = float4x4(
        input.iWorld0,
        input.iWorld1,
        input.iWorld2,
        input.iWorld3
    );
    
    //----------------------------------
    // 法線（ワールド空間）
    //----------------------------------
    float3 N = normalize(mul(float4(input.normal, 0.0f), worldMatrix).xyz);

    //----------------------------------
    // ライティング（VS）
    //----------------------------------
    float3 finalColor = input.color.rgba;

    if (isLighting)
    {
        float3 L = normalize(-lightDir);

        // Wrap Lambert
        float wrap = 0.4f;
        float ndotl = saturate((dot(N, L) + wrap) / (1.0f + wrap));

        // 上限制限（ベタ防止）
        ndotl *= 0.8f;

        finalColor = (ndotl + ambientIntensity) * lightColor * input.color.rgb;
    }

    //----------------------------------
    // 出力
    //----------------------------------
    float4x4 mtx = mul(worldMatrix, ViewProj);
    
    output.position = mul(input.position, mtx);
    output.color = float4(finalColor, input.color.a);
    output.texcoord = input.texcoord;
    output.normal = N;
    output.worldPos = mul(input.position, worldMatrix).xyz;
    output.isLighting = isLighting;
    return output;
}