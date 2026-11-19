
//定数バッファ
float4x4 mtx;

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0; // 頂点カラー
    float2 texcoord : TEXCOORD0;
};
//in → 出力用の変数を示す
//セマンティクス(:)で変数の役割を設定できる
VS_OUTPUT main(in float4 position : POSITION0,
            in float4 color : COLOR,
            in float2 texcoord : TEXCOORD0)
{
    VS_OUTPUT output;
    output.position = mul(position,mtx);
    output.color = color;
    output.texcoord = texcoord;
    return output;
}