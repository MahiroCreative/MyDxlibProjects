// 検証用: 頂点の座標をそのまま射影空間の座標として出す頂点シェーダー。
// DrawPrimitive3DToShader で描画先の全体を覆う板を描き、ピクセルシェーダー側の定数バッファを書き出すのに使う。

struct VS_INPUT
{
	float3 Position : POSITION0;
	float4 SubPosition : POSITION1;
	float3 Normal : NORMAL0;
	float3 Tangent : TANGENT0;
	float3 Binormal : BINORMAL0;
	float4 Diffuse : COLOR0;
	float4 Specular : COLOR1;
	float2 TexCoords0 : TEXCOORD0;
	float2 TexCoords1 : TEXCOORD1;
};

float4 main(VS_INPUT input) : SV_POSITION
{
	return float4(input.Position.xy, 0.5f, 1.0f);
}
