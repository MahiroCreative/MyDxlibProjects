// DepthOfFieldPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/DepthOfField から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】2D の描画(DrawPolygon2DToShader)の入力。DxLib の VS_Shader2D の出力と同じ順番で全部書く(仕様書 3.1)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】2D の描画では DxLib の VS_Shader2D の出力がこの順番で来る
	float4 Diffuse         : COLOR0 ;
	float4 Specular        : COLOR1 ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;

// C++ 側で設定するテクスチャの定義
// 【D3D11】C++ から渡す値(Direct3D 9 版の register( c○ ) の定数の代わり。C++ で CreateShaderConstantBuffer で作り、スロット 4 にセットする)
// packoffset( c○ ) は定数バッファの先頭から何番目の float4 か(Direct3D 9 版の c0 が c0)
cbuffer UserParam : register( b4 )
{
	float2 cfDOF_Focus : packoffset( c0 ) ;		// 被写界深度のフォーカス情報   x:フォーカス開始位置   y:フォーカス終了位置
} ;


// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    ColorMapTexture0 : register( t0 ) ;		// カラーマップテクスチャ( くっきりテクスチャ )
SamplerState ColorMapTexture0Sampler : register( s0 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t1)とサンプラー(s1)を別々に宣言する
Texture2D    ColorMapTexture1 : register( t1 ) ;		// カラーマップテクスチャ( 弱いぼかしのテクスチャ )
SamplerState ColorMapTexture1Sampler : register( s1 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t2)とサンプラー(s2)を別々に宣言する
Texture2D    ColorMapTexture2 : register( t2 ) ;		// カラーマップテクスチャ( 強いぼかしのテクスチャ )
SamplerState ColorMapTexture2Sampler : register( s2 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t3)とサンプラー(s3)を別々に宣言する
Texture2D    DepthMapTexture : register( t3 ) ;		// 深度マップテクスチャ
SamplerState DepthMapTextureSampler : register( s3 ) ;

// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 Color1, Color2 ;
	float BlendRate ;
	float Fade ;
	float Depth ;

	// 深度値を取得する
	Depth = DepthMapTexture.Sample( DepthMapTextureSampler, PSInput.TexCoords0 ) ;

	// フォーカス情報からぼやけ率を算出
	if( Depth < cfDOF_Focus.x )
	{
		Fade = 1.0f - Depth / cfDOF_Focus.x ;
	}
	else
	if( Depth < cfDOF_Focus.y )
	{
		Fade = 0.0f;
	}
	else
	{
		Fade = ( Depth - cfDOF_Focus.y ) / ( 1.0f - cfDOF_Focus.y ) ;
	}

	// ぼやけ率から色を算出
	if( Fade < 0.5f )
	{
		// ぼやけ率が 0.5f 以下の場合はぼかし無し画像と弱いぼかし画像を合成する
		Color1 = ColorMapTexture0.Sample( ColorMapTexture0Sampler, PSInput.TexCoords0 ) ;
		Color2 = ColorMapTexture1.Sample( ColorMapTexture1Sampler, PSInput.TexCoords0 ) ;
		BlendRate = Fade / 0.5f ;
	}
	else
	{
		// ぼやけ率が 0.5f 以上の場合は弱いぼかし画像と強いぼかし画像を合成する
		Color1 = ColorMapTexture1.Sample( ColorMapTexture1Sampler, PSInput.TexCoords0 ) ;
		Color2 = ColorMapTexture2.Sample( ColorMapTexture2Sampler, PSInput.TexCoords0 ) ;
		BlendRate = ( Fade - 0.5f ) / 0.5f ;
	}

	// 合成した色の値を算出
	PSOutput.Color0 = lerp( Color1, Color2, BlendRate ) ;
   
	return PSOutput ;    
}
