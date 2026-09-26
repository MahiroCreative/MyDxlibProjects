// PointLight_DepthShadow_Step2PS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/3DAction_PointLight_DepthShadow から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
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
	float4 DPPosition0     : TEXCOORD1 ;    // 深度バッファ０レンダリング時の座標( 射影空間 )
	float4 DPPosition1     : TEXCOORD2 ;    // 深度バッファ１レンダリング時の座標( 射影空間 )
	float4 DPPosition2     : TEXCOORD3 ;    // 深度バッファ２レンダリング時の座標( 射影空間 )
	float4 DPPosition3     : TEXCOORD4 ;    // 深度バッファ３レンダリング時の座標( 射影空間 )
	float4 DPPosition4     : TEXCOORD5 ;    // 深度バッファ４レンダリング時の座標( 射影空間 )
	float4 DPPosition5     : TEXCOORD6 ;    // 深度バッファ５レンダリング時の座標( 射影空間 )
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
Texture2D    DepthMapTexture0 : register( t1 ) ;		// 深度バッファ０テクスチャ
SamplerState DepthMapTexture0Sampler : register( s1 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t2)とサンプラー(s2)を別々に宣言する
Texture2D    DepthMapTexture1 : register( t2 ) ;		// 深度バッファ１テクスチャ
SamplerState DepthMapTexture1Sampler : register( s2 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t3)とサンプラー(s3)を別々に宣言する
Texture2D    DepthMapTexture2 : register( t3 ) ;		// 深度バッファ２テクスチャ
SamplerState DepthMapTexture2Sampler : register( s3 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t4)とサンプラー(s4)を別々に宣言する
Texture2D    DepthMapTexture3 : register( t4 ) ;		// 深度バッファ３テクスチャ
SamplerState DepthMapTexture3Sampler : register( s4 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t5)とサンプラー(s5)を別々に宣言する
Texture2D    DepthMapTexture4 : register( t5 ) ;		// 深度バッファ４テクスチャ
SamplerState DepthMapTexture4Sampler : register( s5 ) ;
// 【D3D11】sampler の代わりに、テクスチャ(t6)とサンプラー(s6)を別々に宣言する
Texture2D    DepthMapTexture5 : register( t6 ) ;		// 深度バッファ５テクスチャ
SamplerState DepthMapTexture5Sampler : register( s6 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float TextureDepth ;
	float2 DepthTexCoord ;
	float4 DefaultOutput ;
	float3 DPPosition ;


	// テクスチャカラーの読み込み
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0.xy ) ;

	// 出力カラー = ディフューズカラー * テクスチャカラー + スペキュラカラー
	DefaultOutput = PSInput.Diffuse * TextureDiffuseColor + PSInput.Specular ;

	// 出力アルファ = ディフューズアルファ * テクスチャアルファ
	DefaultOutput.a = PSInput.Diffuse.a * TextureDiffuseColor.a ;


	// 深度バッファ０に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition0.x / PSInput.DPPosition0.w;
	DPPosition.y = PSInput.DPPosition0.y / PSInput.DPPosition0.w;
	DPPosition.z = PSInput.DPPosition0.z / PSInput.DPPosition0.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		// 映っている場合は深度バッファ０を適用する

		// 深度バッファ０テクスチャの座標を算出
		// PSInput.DPPosition.xy は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;

		// yは更に上下反転
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;

		// 深度バッファ０テクスチャから深度を取得
		TextureDepth = DepthMapTexture0.Sample( DepthMapTexture0Sampler, DepthTexCoord );

		// テクスチャに記録されている深度( +補正値 )よりＺ値が大きかったら奥にあるということで輝度を半分にする
		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}

	// 深度バッファ１に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition1.x / PSInput.DPPosition1.w;
	DPPosition.y = PSInput.DPPosition1.y / PSInput.DPPosition1.w;
	DPPosition.z = PSInput.DPPosition1.z / PSInput.DPPosition1.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;
		TextureDepth = DepthMapTexture1.Sample( DepthMapTexture1Sampler, DepthTexCoord );

		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}

	// 深度バッファ２に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition2.x / PSInput.DPPosition2.w;
	DPPosition.y = PSInput.DPPosition2.y / PSInput.DPPosition2.w;
	DPPosition.z = PSInput.DPPosition2.z / PSInput.DPPosition2.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;
		TextureDepth = DepthMapTexture2.Sample( DepthMapTexture2Sampler, DepthTexCoord );

		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}

	// 深度バッファ３に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition3.x / PSInput.DPPosition3.w;
	DPPosition.y = PSInput.DPPosition3.y / PSInput.DPPosition3.w;
	DPPosition.z = PSInput.DPPosition3.z / PSInput.DPPosition3.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;
		TextureDepth = DepthMapTexture3.Sample( DepthMapTexture3Sampler, DepthTexCoord );

		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}

	// 深度バッファ４に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition4.x / PSInput.DPPosition4.w;
	DPPosition.y = PSInput.DPPosition4.y / PSInput.DPPosition4.w;
	DPPosition.z = PSInput.DPPosition4.z / PSInput.DPPosition4.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;
		TextureDepth = DepthMapTexture4.Sample( DepthMapTexture4Sampler, DepthTexCoord );

		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}

	// 深度バッファ５に描画座標が映っているかを判定
	DPPosition.x = PSInput.DPPosition5.x / PSInput.DPPosition5.w;
	DPPosition.y = PSInput.DPPosition5.y / PSInput.DPPosition5.w;
	DPPosition.z = PSInput.DPPosition5.z / PSInput.DPPosition5.w;
	if( DPPosition.x > -1.0f && DPPosition.x < 1.0f &&
		DPPosition.y > -1.0f && DPPosition.y < 1.0f &&
		DPPosition.z >  0.0f && DPPosition.z < 1.0f )
	{
		DepthTexCoord.x = ( DPPosition.x + 1.0f ) / 2.0f;
		DepthTexCoord.y = 1.0f - ( DPPosition.y + 1.0f ) / 2.0f;
		TextureDepth = DepthMapTexture5.Sample( DepthMapTexture5Sampler, DepthTexCoord );

		if( DPPosition.z > TextureDepth + 0.001f )
		{
			DefaultOutput.rgb *= 0.5f;
		}
	}


	// 出力カラーをセット
	PSOutput.Color0 = DefaultOutput;


	// 出力パラメータを返す
	return PSOutput ;
}


