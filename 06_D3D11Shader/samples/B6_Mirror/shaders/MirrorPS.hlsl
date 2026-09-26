// MirrorPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/Mirror から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 ProjectionPosition       : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION	// 射影座標
	float4 MirrorProjectionPosition : TEXCOORD0 ;	// 鏡に映る映像の視点での射影座標
	float4 Diffuse                  : COLOR0 ;		// ディフューズカラー
};

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
};


// C++ 側で設定する定数の定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    MirrorTexture : register( t0 ) ;		// 鏡に映すテクスチャ
SamplerState MirrorTextureSampler : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float2 TexCoords ;
	float4 TexColor ;

	// テクスチャ座標の算出
	TexCoords.x = PSInput.MirrorProjectionPosition.x / PSInput.MirrorProjectionPosition.w ;
	TexCoords.y = PSInput.MirrorProjectionPosition.y / PSInput.MirrorProjectionPosition.w ;

	// 鏡に映すテクスチャから色を取得
	TexColor = MirrorTexture.Sample( MirrorTextureSampler, TexCoords ) ;

	// ディフューズカラーと乗算して出力にセット
	PSOutput.Color0 = TexColor * PSInput.Diffuse ;
   
	return PSOutput ;    
}
