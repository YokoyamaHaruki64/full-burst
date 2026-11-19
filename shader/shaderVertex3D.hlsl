
//定数バッファ
cbuffer MatrixBuffer : register(b0)
{
    float4x4 mtx;
    float4x4 worldMatrix;
};

cbuffer LightBuffer : register(b1)
{
    bool isLighting; // ライティングの有効/無効
    float3 pad0;     // パディング用
    
    float3 lightDir; // 光の方向
    float pad1;
    
    float3 lightColor; // 光の色
    float ambientIntensity; // 環境光の強さ
    
};

struct VS_INPUT
{
    float4 position : POSITION0; // 頂点位置
    float4 color : COLOR;        // 頂点カラー
    float2 texcoord : TEXCOORD0; // テクスチャ座標
    float3 normal : NORMAL0;     // 法線ベクトル
};

struct VS_OUTPUT
{
    float4 position : SV_Position; // 変換後の頂点位置
    float4 color : COLOR0;         // 頂点カラー
    float2 texcoord : TEXCOORD0;   // テクスチャ座標
    float3 normal : TEXCOORD1;     // 法線ベクトル
    float3 worldPos : TEXCOORD2; // ワールド座標
    bool isLighting : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

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
    output.position = mul(input.position, mtx);
    output.color = float4(finalColor, input.color.a);
    output.texcoord = input.texcoord;
    output.normal = N;
    output.worldPos = mul(input.position, worldMatrix).xyz;
    output.isLighting = isLighting;
    return output;
}
    /* メモ:法線ベクトルの回転
    オブジェクトを回転しても法線ベクトルは回転しない。
    よって、法線ベクトルも回転させる必要がある。
    mtxは回転行列を含んでいる。そのため、return mul(...)で頂点位置は正確に変換されるが、
    法線ベクトルは変換されない。
    normal = mul(float4(normal, 0.0), mtx).xyz;
    で回転させることが出来る。
    が、ビュー行列やプロジェクション行列が含まれていると正確に回転できない。
    そのため、オブジェクトのワールド行列とカメラの行列を分けて管理する必要がある。
    */
