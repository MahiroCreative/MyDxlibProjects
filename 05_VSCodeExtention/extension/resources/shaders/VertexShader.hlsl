// 頂点シェーダー(Direct3D 11 / vs_4_0)。3D の VERTEX3DSHADER 用。
//
// 使い方(C++ 側):
//   int vs = LoadVertexShader("shaders/bin/__SHADER_NAME__.vso");
//   SetUseVertexShader(vs);
//   SetUsePixelShader(ピクセルシェーダー);
//   DrawPolygon3DToShader(頂点, ポリゴン数);
//
// カメラやモデルの行列は DxLib が定数バッファ b1 に入れて渡してくる。
// 並びは DxLib 3.24f のソース(Shader/Windows/Direct3D11/DxShader_VS_D3D11.h)と同じ。

// DxLib が設定する基本パラメータ(先頭から必要な分だけを宣言している)
cbuffer DxLibVSBase : register(b1)
{
	float4 g_AntiViewportMatrix[4]; // 2D 用(3D では使わない)
	float4 g_ProjectionMatrix[4];   // ビュー → プロジェクション
	float4 g_ViewMatrix[3];         // ワールド → ビュー
	float4 g_LocalWorldMatrix[3];   // ローカル → ワールド(MV1SetMatrix などの結果)
};

// VERTEX3DSHADER の全要素。使わない要素も消さず、この順番のまま宣言すること。
// Direct3D 11 は頂点データを「入力の何番目か」で渡すので、途中を省くと値がずれて何も描かれない。
struct VS_INPUT
{
	float3 Position    : POSITION0; // pos
	float4 SubPosition : POSITION1; // spos
	float3 Normal      : NORMAL0;   // norm
	float3 Tangent     : TANGENT0;  // tan
	float3 Binormal    : BINORMAL0; // binorm
	float4 Diffuse     : COLOR0;    // dif
	float4 Specular    : COLOR1;    // spc
	float2 TexCoords0  : TEXCOORD0; // u, v
	float2 TexCoords1  : TEXCOORD1; // su, sv
};

// ピクセルシェーダーの PS_INPUT と同じ並びにする
struct VS_OUTPUT
{
	float4 Position   : SV_POSITION;
	float4 Diffuse    : COLOR0;
	float2 TexCoords0 : TEXCOORD0;
	float2 TexCoords1 : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	// ローカル → ワールド → ビュー → プロジェクション
	float4 local = float4(input.Position, 1.0f);
	float4 world = float4(dot(local, g_LocalWorldMatrix[0]), dot(local, g_LocalWorldMatrix[1]), dot(local, g_LocalWorldMatrix[2]), 1.0f);
	float4 view = float4(dot(world, g_ViewMatrix[0]), dot(world, g_ViewMatrix[1]), dot(world, g_ViewMatrix[2]), 1.0f);
	output.Position = float4(dot(view, g_ProjectionMatrix[0]), dot(view, g_ProjectionMatrix[1]), dot(view, g_ProjectionMatrix[2]), dot(view, g_ProjectionMatrix[3]));

	output.Diffuse = input.Diffuse;
	output.TexCoords0 = input.TexCoords0;
	output.TexCoords1 = input.TexCoords1;
	return output;
}
