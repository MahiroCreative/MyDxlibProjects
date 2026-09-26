// 2D の絵を頂点シェーダーで動かす(旗のようになびかせる)。Direct3D 11 / vs_4_0
// C++ 側で Camera2D.h の SetupCamera2D() を呼び、DrawPolygon3DToShader で描く。
// 板の頂点の座標は画面の画素の座標(x は右、y は下)。

#include "DxLibVS.hlsli"

// C++ から渡す値(CreateShaderConstantBuffer で作り、頂点シェーダーのスロット 4 に置く)
cbuffer WaveParam : register(b4)
{
	float g_Time;		// 経過時間(秒)
	float g_Amplitude;	// ゆれの大きさ(画素)。0 ならゆれない
	float g_WaveLength;	// 波の長さ(画素)
	float g_Speed;		// 波が進む速さ(1 秒あたりの角度。ラジアン)
};

// DrawPolygon3DToShader の頂点(VERTEX3DSHADER)。使わない要素も省かず、この順番で全部書く
struct VS_INPUT
{
	float3 Position   : POSITION0;	// pos(画面の画素の座標)
	float4 SubPosition: POSITION1;	// spos
	float3 Normal     : NORMAL0;	// norm
	float3 Tangent    : TANGENT0;	// tan
	float3 Binormal   : BINORMAL0;	// binorm
	float4 Diffuse    : COLOR0;		// dif
	float4 Specular   : COLOR1;		// spc
	float2 TexCoords0 : TEXCOORD0;	// u, v
	float2 TexCoords1 : TEXCOORD1;	// su, sv
};

// ピクセルシェーダーに渡す値(Sprite2DPS.hlsl の PS_INPUT と同じ並び)
struct VS_OUTPUT
{
	float4 Position   : SV_POSITION;
	float4 Diffuse    : COLOR0;
	float2 TexCoords0 : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float3 pos = input.Position;

	// 【課題 6-2】スライムのぷるぷる: 下の端(v = 1)を留めて、縦に伸びたら横に縮む、縦に縮んだら横に伸びる
	float wave = sin(g_Time * g_Speed);
	pos.y -= (1.0f - input.TexCoords0.y) * wave * g_Amplitude;			// 上へ行くほど大きく上下に
	pos.x -= (input.TexCoords0.x - 0.5f) * wave * g_Amplitude * 0.5f;	// 真ん中から左右に(縦と逆向き)

	// 画面の画素の座標 → ビュー → 射影(カメラは SetupCamera2D で設定したもの)
	float4 world = float4(pos, 1.0f);
	float4 view;
	view.x = dot(world, g_Base.ViewMatrix[0]);
	view.y = dot(world, g_Base.ViewMatrix[1]);
	view.z = dot(world, g_Base.ViewMatrix[2]);
	view.w = 1.0f;
	output.Position.x = dot(view, g_Base.ProjectionMatrix[0]);
	output.Position.y = dot(view, g_Base.ProjectionMatrix[1]);
	output.Position.z = dot(view, g_Base.ProjectionMatrix[2]);
	output.Position.w = dot(view, g_Base.ProjectionMatrix[3]);

	output.Diffuse = input.Diffuse;
	output.TexCoords0 = input.TexCoords0;
	return output;
}
