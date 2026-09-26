// SkinMesh4_DirSpotPointLight_ToonPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/16_SkinMesh4_DirSpotPointLight_Toon から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION         // 座標( プロジェクション空間 )
	float4 Diffuse         : COLOR0 ;           // ディフューズカラー
	float4 Specular        : COLOR1 ;           // スペキュラカラー
	float2 TexCoords0      : TEXCOORD0 ;        // テクスチャ座標
	float2 ToonCoords0     : TEXCOORD1 ;        // トゥーンテクスチャ座標
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;		// ディフューズマップテクスチャ
SamplerState DiffuseMapTextureSampler : register( s0 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t1)とサンプラー(s1)を別々に宣言する
Texture2D    ToonTexture : register( t1 ) ;		// トゥーンテクスチャ
SamplerState ToonTextureSampler : register( s1 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 ToonColor ;

	// テクスチャカラーの取得
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0 ) ;

	// トゥーンテクスチャカラーを取得
	ToonColor = ToonTexture.Sample( ToonTextureSampler, PSInput.ToonCoords0 ) ;


	// 出力 = saturate( ディフューズカラー * トゥーンテクスチャカラー + スペキュラカラー ) * テクスチャカラー
	PSOutput.Color0.rgb = saturate( PSInput.Diffuse.rgb * ToonColor.rgb + PSInput.Specular.rgb ) * TextureDiffuseColor.rgb ;

	// アルファ値 = ディフューズアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * PSInput.Diffuse.a * g_Base.FactorColor.a ;


	// 出力パラメータを返す
	return PSOutput ;
}
