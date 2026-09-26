// NormalMesh_DirSpotPointLight_NrmMapPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/21_NormalMesh_DirSpotPointLight_NrmMap から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
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
	float3 VTan            : TEXCOORD2 ;    // 接線( ビュー空間 )
	float3 VBin            : TEXCOORD3 ;    // 従法線( ビュー空間 )
	float3 VNormal         : TEXCOORD4 ;    // 法線( ビュー空間 )
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
Texture2D    NormalMapTexture : register( t1 ) ;		// 法線マップテクスチャ
SamplerState NormalMapTextureSampler : register( s1 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float3 V_to_Eye ;
	float3 Normal ;
	float3 VNrm ;
	float3 VTan ;
	float3 VBin ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float4 SpecularColor ;
	float3 TempF3 ;
	float Temp ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightDirectionCosA ;
	float lLightGen ;
	float3 lLightDir ;
	float DiffuseAngleGen ;

	// 接線・従法線・法線を正規化
	VNrm = normalize( PSInput.VNormal );
	VTan = normalize( PSInput.VTan );
	VBin = normalize( PSInput.VBin );

	// 頂点座標から視点へのベクトルを接底空間に投影した後正規化して保存
	TempF3.x = dot( VTan, -PSInput.VPosition.xyz ) ;
	TempF3.y = dot( VBin, -PSInput.VPosition.xyz ) ;
	TempF3.z = dot( VNrm, -PSInput.VPosition.xyz ) ;
	V_to_Eye = normalize( TempF3 ) ;

	// 法線の 0～1 の値を -1.0～1.0 に変換する
	Normal = ( NormalMapTexture.Sample( NormalMapTextureSampler, PSInput.TexCoords0.xy ).rgb - float3( 0.5f, 0.5f, 0.5f ) ) * 2.0f ;

	// ディフューズカラーとスペキュラカラーの蓄積値を初期化
	TotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	TotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;


	// ディレクショナルライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	TempF3 = g_Common.Light[ 0 ].Direction ;

	// ライトのベクトルを接地空間に変換
	lLightDir.x = dot( VTan, TempF3 ) ;
	lLightDir.y = dot( VBin, TempF3 ) ;
	lLightDir.z = dot( VNrm, TempF3 ) ;

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


	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	TempF3 = normalize( PSInput.VPosition.xyz - g_Common.Light[ 1 ].Position ) ;

	// ライトのベクトルを接地空間に変換
	lLightDir.x = dot( VTan, TempF3 ) ;
	lLightDir.y = dot( VBin, TempF3 ) ;
	lLightDir.z = dot( VNrm, TempF3 ) ;


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
	TempF3.x = dot( VTan, g_Common.Light[ 1 ].Direction ) ;
	TempF3.y = dot( VBin, g_Common.Light[ 1 ].Direction ) ;
	TempF3.z = dot( VNrm, g_Common.Light[ 1 ].Direction ) ;
	lLightDirectionCosA = dot( lLightDir, TempF3 ) ;

	// スポットライト減衰計算  pow( falloff, ( ( Cos a - Cos f ) / ( Cos q - Cos f ) ) )
	lLightGen *= saturate( pow( abs( max( lLightDirectionCosA - g_Common.Light[ 1 ].SpotParam0, 0.0f ) * g_Common.Light[ 1 ].SpotParam1 ), g_Common.Light[ 1 ].FallOff ) ) ;

	// --------------------------------


	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ 1 ].RangePow2 ) ;

	// 距離・スポットライト減衰値計算 =======================================( 終了 )


	// ディフューズ色計算

	// ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * マテリアルディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
	TotalDiffuse += ( float4( g_Common.Light[ 1 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse * DiffuseAngleGen + g_Common.Light[ 1 ].Ambient ) * lLightGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	TotalSpecular += Temp * lLightGen.x * float4( g_Common.Light[ 1 ].Specular, 0.0f ) ;

	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// ポイントライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	TempF3 = normalize( PSInput.VPosition.xyz - g_Common.Light[ 2 ].Position ) ;

	// ライトのベクトルを接地空間に変換
	lLightDir.x = dot( VTan, TempF3 ) ;
	lLightDir.y = dot( VBin, TempF3 ) ;
	lLightDir.z = dot( VNrm, TempF3 ) ;


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
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * マテリアルディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
	TotalDiffuse += ( float4( g_Common.Light[ 2 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse * DiffuseAngleGen + g_Common.Light[ 2 ].Ambient ) * lLightGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	TotalSpecular += Temp * lLightGen.x * float4( g_Common.Light[ 2 ].Specular, 0.0f ) ;

	// ポイントライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
	TotalDiffuse += g_Common.Material.Ambient_Emissive ;

	// SpecularColor = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	SpecularColor = TotalSpecular * g_Common.Material.Specular ;

	// 出力カラー = TotalDiffuse * テクスチャカラー + SpecularColor
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0.xy ) ;
	PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;

	// アルファ値 = テクスチャアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * g_Common.Material.Diffuse.a * g_Base.FactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}
