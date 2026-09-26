// NormalMesh_NoLight_FogPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/03_NormalMesh_NoLight_Fog から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION
	float2 TexCoords0      : TEXCOORD0 ;
	float  Fog : TEXCOORD1 ;	// フォグ【D3D11】固定機能のフォグは無いので、ピクセルシェーダーで混ぜる
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;		// ディフューズマップテクスチャ
SamplerState DiffuseMapTextureSampler : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;

	// テクスチャカラーの読み込み
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0 ) ;

	// 出力カラー = テクスチャカラー
	PSOutput.Color0 = TextureDiffuseColor ;

	// 出力アルファ = テクスチャアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * g_Base.FactorColor.a ;

	// 【D3D11】フォグの色を混ぜる(Direct3D 9 版は固定機能のフォグが頂点シェーダーの出力 FOG を使って混ぜていた)
	PSOutput.Color0.rgb = lerp( g_Common.Fog.Color.rgb, PSOutput.Color0.rgb, saturate( PSInput.Fog ) ) ;

	// 出力パラメータを返す
	return PSOutput ;
}
