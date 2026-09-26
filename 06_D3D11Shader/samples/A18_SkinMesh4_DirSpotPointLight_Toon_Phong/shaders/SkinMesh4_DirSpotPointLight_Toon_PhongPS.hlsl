// SkinMesh4_DirSpotPointLight_Toon_PhongPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/18_SkinMesh4_DirSpotPointLight_Toon_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

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
// 【D3D11】sampler の代わりに、テクスチャ(t1)とサンプラー(s1)を別々に宣言する
Texture2D    ToonTexture : register( t1 ) ;		// トゥーンテクスチャ
SamplerState ToonTextureSampler : register( s1 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 lTextureDiffuseColor ;
	float4 lSpecularColor ;
	float4 lDiffuseColor ;
	float3 Normal ;
	float lDiffuseAngleGen ;
	float4 lTotalDiffuse ;
	float4 lTotalSpecular ;
	float4 lTotalAmbient ;
	float4 lToonColor ;
	float lTotalLightGen ;
	float3 V_to_Eye ;
	float3 TempF3 ;
	float Temp ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightGen ;
	float3 lLightDir ;
	float lLightDirectionCosA ;


	// 法線の準備
	Normal = normalize( PSInput.VNormal ) ;

	// 頂点座標から視点へのベクトルを正規化
	V_to_Eye = normalize( -PSInput.VPosition ) ;

	// ディフューズカラーとスペキュラカラーとアンビエントカラーの合計値を初期化
	lTotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	lTotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	lTotalAmbient  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;

	// ライトの減衰率合計値の初期化
	lTotalLightGen = 0.0f ;


	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルのセット
	lLightDir = g_Common.Light[ 0 ].Direction ;


	// ディフューズ角度減衰率計算
	lDiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズ減衰率を合計値に加算
	lTotalLightGen += lDiffuseAngleGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラライト合計値 += スペキュラ角度減衰計算結果 * ライトのスペキュラカラー
	lTotalSpecular += Temp * float4( g_Common.Light[ 0 ].Specular, 0.0f ) ;


	// ライトのディフューズカラーを合計値に加算
	lTotalDiffuse  += float4( g_Common.Light[ 0 ].Diffuse, 0.0f ) ;

	// ライトのアンビエントカラーを合計値に加算
	lTotalAmbient  += g_Common.Light[ 0 ].Ambient ;

	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( PSInput.VPosition.xyz - g_Common.Light[ 1 ].Position ) ;


	// 距離・スポットライト減衰値計算 =======================================( 開始 )

	// 距離減衰計算 ------------------

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = PSInput.VPosition.xyz - g_Common.Light[ 1 ].Position ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( g_Common.Light[ 1 ].Attenuation0 + g_Common.Light[ 1 ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ 1 ].Attenuation2 * lLightDistancePow2 ) ;

	// --------------------------------


	// スポットライト減衰計算 --------

	// ライト方向ベクトルとライト位置から頂点位置へのベクトルの内積( 即ち Cos a )を計算 
	lLightDirectionCosA = dot( lLightDir, g_Common.Light[ 1 ].Direction ) ;

	// スポットライト減衰計算  pow( falloff, ( ( Cos a - Cos f ) / ( Cos q - Cos f ) ) )
	lLightGen *= saturate( pow( abs( max( lLightDirectionCosA - g_Common.Light[ 1 ].SpotParam0, 0.0f ) * g_Common.Light[ 1 ].SpotParam1 ), g_Common.Light[ 1 ].FallOff ) ) ;

	// --------------------------------


	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ 1 ].RangePow2 ) ;

	// 距離・スポットライト減衰値計算 =======================================( 終了 )


	// ディフューズ角度減衰率計算
	lDiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズ減衰率を合計値に加算
	lTotalLightGen += lDiffuseAngleGen * lLightGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラライト合計値 += スペキュラ角度減衰計算結果 * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	lTotalSpecular += Temp * lLightGen * float4( g_Common.Light[ 1 ].Specular, 0.0f ) ;


	// ライトのディフューズカラーを合計値に加算
	lTotalDiffuse  += float4( g_Common.Light[ 1 ].Diffuse, 0.0f ) ;

	// ライトのアンビエントカラーを合計値に加算
	lTotalAmbient  += g_Common.Light[ 1 ].Ambient ;

	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// ポイントライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( PSInput.VPosition.xyz - g_Common.Light[ 2 ].Position ) ;


	// 距離減衰値計算 =======================================================( 開始 )

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = PSInput.VPosition.xyz - g_Common.Light[ 2 ].Position ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( g_Common.Light[ 2 ].Attenuation0 + g_Common.Light[ 2 ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ 2 ].Attenuation2 * lLightDistancePow2 ) ;

	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ 2 ].RangePow2 ) ;

	// 距離減衰値計算 =======================================================( 終了 )


	// ディフューズ色計算

	// ディフューズ角度減衰率計算
	lDiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズ減衰率を合計値に加算
	lTotalLightGen += lDiffuseAngleGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラライト合計値 += スペキュラ角度減衰計算結果 * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	lTotalSpecular += Temp * lLightGen * float4( g_Common.Light[ 2 ].Specular, 0.0f ) ;


	// ライトのディフューズカラーを合計値に加算
	lTotalDiffuse  += float4( g_Common.Light[ 2 ].Diffuse, 0.0f ) ;

	// ライトのアンビエントカラーを合計値に加算
	lTotalAmbient  += g_Common.Light[ 2 ].Ambient ;

	// ポイントライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )

	// アンビエントカラーの蓄積値 += マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの
	lTotalAmbient += g_Common.Material.Ambient_Emissive ;


	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// テクスチャカラーの取得
	lTextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0 ) ;

	// トゥーンテクスチャカラーをライトのディフューズ減衰率から取得
	lToonColor = ToonTexture.Sample( ToonTextureSampler, lTotalLightGen ) ;

	// ディフューズカラー = ライトのディフューズカラー蓄積値 * マテリアルのディフューズカラー
	lDiffuseColor  = lTotalDiffuse  * g_Common.Material.Diffuse ;
	
	// スペキュラカラー = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	lSpecularColor = lTotalSpecular * g_Common.Material.Specular ;

	// 出力 = saturate( saturate( ディフューズカラー * アンビエントカラーの蓄積値 ) * トゥーンテクスチャカラー + スペキュラカラー ) * テクスチャカラー
	PSOutput.Color0.rgb = saturate( saturate( lDiffuseColor.rgb + lTotalAmbient.rgb ) * lToonColor.rgb + lSpecularColor.rgb ) * lTextureDiffuseColor.rgb ;

	// アルファ値 = ディフューズアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = lTextureDiffuseColor.a * g_Common.Material.Diffuse.a * g_Base.FactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}
