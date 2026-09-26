// SkinMesh4_OpticalCamouflagePS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/3DAction_OpticalCamouflage から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION         // 座標( 射影空間 )
	float4 PPosition       : TEXCOORD0 ;        // 座標( 射影空間 )
	float3 VNormal         : TEXCOORD1 ;        // 法線( ビュー空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定するテクスチャの定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    TempDrawScreenTexture : register( t0 ) ;		// ステージモデルを描画した画像
SamplerState TempDrawScreenTextureSampler : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureColor ;
	float2 TexCoord ;
	float4 PPositionN ;


	// PPosition の xyz を w で割って非同次座標にする
	PPositionN = PSInput.PPosition / PSInput.PPosition.w ;

	
	// 画面テクスチャの座標を算出
	// PPositionN.xy は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
	TexCoord.x = ( PPositionN.x + 1.0f ) / 2.0f;

	// yは更に上下反転
	TexCoord.y = 1.0f - ( PPositionN.y + 1.0f ) / 2.0f;


	// テクスチャ座標を法線方向に少しずらす
	TexCoord += PSInput.VNormal.xy * 0.01f ;


	// テクスチャカラーの読み込み
	TextureColor = TempDrawScreenTexture.Sample( TempDrawScreenTextureSampler, TexCoord ) ;


	// 出力カラーはテクスチャカラーを少しだけ暗くしたもの
	PSOutput.Color0.rgb = TextureColor.rgb * 0.9f ;

	// 出力アルファ値は1.0fで固定
	PSOutput.Color0.a = 1.0f ;

	// 出力パラメータを返す
	return PSOutput ;
}


