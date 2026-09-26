// DirLight_DepthShadow_Step2PS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/3DAction_DepthShadow から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION     // 座標( 射影空間 )
	float4 Diffuse         : COLOR0 ;       // ディフューズカラー
	float4 Specular        : COLOR1 ;       // スペキュラカラー
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float4 LPPosition      : TEXCOORD1 ;    // ライトからみた座標( ライトの射影空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定する定数やテクスチャの定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;		// ディフューズマップテクスチャ
SamplerState DiffuseMapTextureSampler : register( s0 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t1)とサンプラー(s1)を別々に宣言する
Texture2D    DepthMapTexture : register( t1 ) ;		// 深度バッファテクスチャ
SamplerState DepthMapTextureSampler : register( s1 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float TextureDepth ;
	float2 DepthTexCoord ;
	float4 DefaultOutput ;


	// テクスチャカラーの読み込み
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0.xy ) ;

	// 出力カラー = ディフューズカラー * テクスチャカラー + スペキュラカラー
	DefaultOutput = PSInput.Diffuse * TextureDiffuseColor + PSInput.Specular ;

	// 出力アルファ = ディフューズアルファ * テクスチャアルファ
	DefaultOutput.a = PSInput.Diffuse.a * TextureDiffuseColor.a ;


	// 深度テクスチャの座標を算出
	// PSInput.LPPosition.xy は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
	DepthTexCoord.x = ( PSInput.LPPosition.x + 1.0f ) / 2.0f;

	// yは更に上下反転
	DepthTexCoord.y = 1.0f - ( PSInput.LPPosition.y + 1.0f ) / 2.0f;

	// 深度バッファテクスチャから深度を取得
	TextureDepth = DepthMapTexture.Sample( DepthMapTextureSampler, DepthTexCoord );

	// テクスチャに記録されている深度( +補正値 )よりＺ値が大きかったら奥にあるということで輝度を半分にする
	if( PSInput.LPPosition.z > TextureDepth + 25.0f )
	{
		DefaultOutput.rgb *= 0.2f;	// 【課題 10-2】影の所の明るさ。0.5(半分)→ 0.2 で濃くする
	}


	// 出力カラーをセット
	PSOutput.Color0 = DefaultOutput;


	// 出力パラメータを返す
	return PSOutput ;
}


