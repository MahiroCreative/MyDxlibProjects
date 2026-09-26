// DepthShadow_Step1PS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/3DAction_PointLight_DepthShadow から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION         // 座標( 射影空間 )
	float4 PPosition       : TEXCOORD0 ;        // 座標( 射影空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;

// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;

	// Ｚ値を色として出力
	PSOutput.Color0 = PSInput.PPosition.z / PSInput.PPosition.w;

	// 透明にならないようにアルファは必ず１
	PSOutput.Color0.a = 1.0f;

	// 出力パラメータを返す
	return PSOutput ;
}


