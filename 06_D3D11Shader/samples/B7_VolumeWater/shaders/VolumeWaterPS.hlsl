// VolumeWaterPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/B_applied/VolumeWater から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 ProjectionPosition       : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION	// 座標( 射影空間 )
	float4 ViewPosition             : TEXCOORD0 ;	// 座標( ビュー空間 )
	float4 ProjectionPositionSub    : TEXCOORD1 ;	// 座標( 射影空間 )ピクセルシェーダーで参照する為の物
	float4 Diffuse                  : COLOR0 ;		// ディフューズカラー
};

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0 : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
};


// C++ 側で設定する定数の定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DepthTexture : register( t0 ) ;   // 深度テクスチャ
SamplerState DepthTextureSampler : register( s0 ) ;

// 【D3D11】C++ から渡す値(Direct3D 9 版の register( c○ ) の定数の代わり。C++ で CreateShaderConstantBuffer で作り、スロット 4 にセットする)
// packoffset( c○ ) は定数バッファの先頭から何番目の float4 か(Direct3D 9 版の c0 が c0)
cbuffer UserParam : register( b4 )
{
	float MaxOpacityDistance : packoffset( c0 ) ;   // 不透明度が最大になる水中の距離
	float MinOpacity : packoffset( c1 ) ;   // 最低不透明度
	float2 DepthTextureUseSize : packoffset( c2 ) ;	// 深度テクスチャで使用しているサイズ
} ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float2 TexCoords ;
	float4 Depth ;
	float refractiveRatio ;

	// 深度テクスチャ座標の算出
	TexCoords.x = (  PSInput.ProjectionPositionSub.x / PSInput.ProjectionPositionSub.w + 1.0f ) / 2.0f ;
	TexCoords.y = ( -PSInput.ProjectionPositionSub.y / PSInput.ProjectionPositionSub.w + 1.0f ) / 2.0f ;

	// 深度テクスチャから深度を取得
	Depth = DepthTexture.Sample( DepthTextureSampler, TexCoords * DepthTextureUseSize ) ;

	// 不透明度を計算
	PSOutput.Color0.a = saturate( ( 1.0f - MinOpacity ) * ( Depth.r - PSInput.ViewPosition.z ) / MaxOpacityDistance + MinOpacity ) ;

	// 色のセット
	PSOutput.Color0.rgb = PSInput.Diffuse.rgb ;
   
	return PSOutput ;
}
