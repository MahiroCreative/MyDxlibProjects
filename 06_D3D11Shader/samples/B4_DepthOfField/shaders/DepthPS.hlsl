// DepthPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/DepthOfField から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
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


// C++ 側で設定する定数の定義
// 【D3D11】C++ から渡す値(Direct3D 9 版の register( c○ ) の定数の代わり。C++ で CreateShaderConstantBuffer で作り、スロット 4 にセットする)
// packoffset( c○ ) は定数バッファの先頭から何番目の float4 か(Direct3D 9 版の c0 が c0)
cbuffer UserParam : register( b4 )
{
	float3 cfDOF_Area : packoffset( c0 ) ;	// 被写界深度の範囲情報   x:被写界深度開始位置   y:被写界深度終了位置   z:被写界深度の範囲の逆数
} ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float z_param ;

	// 被写界深度の範囲内を 0.0f ～ 1.0f に変換
	if( PSInput.ViewPosition.z < cfDOF_Area.x )
	{
		PSOutput.Color0.r = 0.0f ;
	}
	else
	if( PSInput.ViewPosition.z > cfDOF_Area.y )
	{
		PSOutput.Color0.r = 1.0f ;
	}
	else
	{
		PSOutput.Color0.r = ( PSInput.ViewPosition.z - cfDOF_Area.x ) * cfDOF_Area.z ;
	}

	PSOutput.Color0.g = 0.0f ;
	PSOutput.Color0.b = 0.0f ;
	// 【D3D11】原本はアルファを 0 にしていたが、今の DxLib(3.24f)はモデルをアルファブレンドで描くので、アルファ 0 では何も書き込まれない
	// (Direct3D 9 でも同じ。深度値の画像が真っ黒のままになり、画面全体がぼける)。書き込まれるよう 1 にする
	PSOutput.Color0.a = 1.0f ;
   
	return PSOutput ;    
}
