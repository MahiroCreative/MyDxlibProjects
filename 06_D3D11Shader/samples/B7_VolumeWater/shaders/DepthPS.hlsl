// DepthPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/VolumeWater から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 ProjectionPosition : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION		// 射影座標
	float4 ViewPosition       : TEXCOORD0 ;		// ビュー座標
};

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
};


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;

	// 奥行き値を書き込み
	PSOutput.Color0.r = PSInput.ViewPosition.z ;
	PSOutput.Color0.g = 0.0f ;
	PSOutput.Color0.b = 0.0f ;
	PSOutput.Color0.a = 1.0 ;
   
	return PSOutput ;    
}
