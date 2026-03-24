SamplerState smp : register(s0);
Texture2D<float4> tex : register(t0);
Texture2D<float4> norm : register(t1);
// ↑頂点シェーダから通ってくる画像(モデル本体が持っている画像)
// ↓自分が渡している画像

struct PS_INPUT
{
    float4 Diffuse : COLOR0; // ディフューズカラー
    float4 Specular : COLOR1; // スペキュラカラー
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 TexCoords0 : TEXCOORD0; // テクスチャ座標
    float4 svPosition : SV_POSITION; // 座標( プロジェクション空間 )
    float4 Position : POSITION0; // 座標( ワールド空間 )
    float4 P : POSITION1; // 座標( プロジェクション空間 )
    float4 VPosition : POSITION2; // 座標( ワールド空間 * ビュー )
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 depht : SV_Target1;
};

PS_OUTPUT main(PS_INPUT input) : SV_TARGET
{
    PS_OUTPUT outdata;
    
    // 今までの通常描画部分
    outdata.color = tex.Sample(smp, input.TexCoords0);
    // 深度値
    float dep = input.P.z / input.P.w; // wで割ってあげないと0～1にならない
    dep = pow(dep, 20);
    outdata.depht = float4(dep, dep, dep, 1.0);
    
    return outdata;
}