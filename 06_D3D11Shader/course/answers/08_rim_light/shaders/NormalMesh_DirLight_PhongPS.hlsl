// NormalMesh_DirLight_PhongPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/11_NormalMesh_DirLight_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION     // 座標( プロジェクション空間 )
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VNormal         : TEXCOORD2 ;    // 法線( ビュー空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定するテクスチャや定数の定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;		// ディフューズマップテクスチャ
SamplerState DiffuseMapTextureSampler : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 SpecularColor ;
	float3 Normal ;
	float DiffuseAngleGen ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float3 TempF3 ;
	float Temp ;
	float3 V_to_Eye ;
	float3 lLightDir ;


	// 法線の準備
	Normal = normalize( PSInput.VNormal ) ;

	// 頂点座標から視点へのベクトルを正規化
	V_to_Eye = normalize( -PSInput.VPosition ) ;

	// ディフューズカラーとスペキュラカラーの蓄積値を初期化
	TotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	TotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;


	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルのセット
	lLightDir = g_Common.Light[ 0 ].Direction ;

	// ディフューズ色計算

	// DiffuseAngleGen = ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ライトのディフューズカラー * マテリアルのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
	TotalDiffuse += float4( g_Common.Light[ 0 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse * DiffuseAngleGen + g_Common.Light[ 0 ].Ambient ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラカラー蓄積値 += Temp * ライトのスペキュラカラー
	TotalSpecular += Temp * float4( g_Common.Light[ 0 ].Specular, 0.0f ) ;

	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
	TotalDiffuse += g_Common.Material.Ambient_Emissive ;

	// SpecularColor = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	SpecularColor = TotalSpecular * g_Common.Material.Specular ;

	// 出力カラー = TotalDiffuse * テクスチャカラー + SpecularColor
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0.xy ) ;
	PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;

	// 【課題 8-2】リムライト: 視線と法線が直角に近い所(輪郭)ほど明るくする。
	// dot( 法線, 視点への向き ) は正面で 1、輪郭で 0。1 から引いて 3 乗し、輪郭の近くだけに絞る
	float lRim = pow( 1.0f - saturate( dot( Normal, V_to_Eye ) ), 3.0f ) ;
	PSOutput.Color0.rgb += lRim * float3( 0.6f, 0.8f, 1.0f ) ;

	// アルファ値 = テクスチャアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * g_Common.Material.Diffuse.a * g_Base.FactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}
