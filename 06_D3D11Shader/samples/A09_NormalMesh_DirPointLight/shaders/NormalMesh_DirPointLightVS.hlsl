// NormalMesh_DirPointLightVS.fx の Direct3D 11 版(vs_4_0。剛体メッシュ)
// reference/d3d9_original/A_base/09_NormalMesh_DirPointLight から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibVS.hlsli"

// 頂点シェーダーの入力
// 【D3D11】DxLib の MV1 の頂点(剛体メッシュ)の並び。使わない要素も省かず、この順番で書く(仕様書 5.3)
struct VS_INPUT
{
	float3 Position        : POSITION0     ;	// 座標( ローカル空間 )【D3D11】float3
	float3 Normal          : NORMAL0       ;	// 法線( ローカル空間 )
	float4 Diffuse         : COLOR0        ;	// ディフューズカラー
	float4 Specular        : COLOR1        ;	// スペキュラカラー
	float4 TexCoords0      : TEXCOORD0     ;	// テクスチャ座標( u, v, 1, 1 )
	float4 TexCoords1      : TEXCOORD1     ;	// サブテクスチャ座標【D3D11】使わなくても書く
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION
	float4 Diffuse         : COLOR0 ;
	float4 Specular        : COLOR1 ;
	float2 TexCoords0      : TEXCOORD0 ;
} ;


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;
	float3 lWorldNrm ;
	float3 lViewNrm ;
	float3 lLightHalfVec ;
	float4 lLightLitParam ;
	float4 lLightLitDest ;
	float3 lLightDir ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightGen ;
	float4 lTotalDiffuse ;
	float4 lTotalSpecular ;


	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ローカル座標をワールド座標に変換
	lWorldPosition.x = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 0 ] ) ;
	lWorldPosition.y = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 1 ] ) ;
	lWorldPosition.z = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 2 ] ) ;
	lWorldPosition.w = 1.0f ;

	// ワールド座標をビュー座標に変換
	lViewPosition.x = dot( lWorldPosition, g_Base.ViewMatrix[ 0 ] ) ;
	lViewPosition.y = dot( lWorldPosition, g_Base.ViewMatrix[ 1 ] ) ;
	lViewPosition.z = dot( lWorldPosition, g_Base.ViewMatrix[ 2 ] ) ;
	lViewPosition.w = 1.0f ;

	// ビュー座標を射影座標に変換
	VSOutput.Position.x = dot( lViewPosition, g_Base.ProjectionMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( lViewPosition, g_Base.ProjectionMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( lViewPosition, g_Base.ProjectionMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( lViewPosition, g_Base.ProjectionMatrix[ 3 ] ) ;

	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// ライトの処理 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ディフューズカラーとスペキュラカラーの蓄積値の初期化
	lTotalDiffuse  = float4( 0, 0, 0, 0 ) ;
	lTotalSpecular = float4( 0, 0, 0, 0 ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 開始 )

	// ローカルベクトルをワールドベクトルに変換
	lWorldNrm.x = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 0 ].xyz ) ;
	lWorldNrm.y = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 1 ].xyz ) ;
	lWorldNrm.z = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 2 ].xyz ) ;

	// ワールドベクトルをビューベクトルに変換
	lViewNrm.x = dot( lWorldNrm, g_Base.ViewMatrix[ 0 ].xyz ) ;
	lViewNrm.y = dot( lWorldNrm, g_Base.ViewMatrix[ 1 ].xyz ) ;
	lViewNrm.z = dot( lWorldNrm, g_Base.ViewMatrix[ 2 ].xyz ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 終了 )


	// ディレクショナルライトの処理 *****************************************************( 開始 )

	// ライトの方向セット
	lLightDir = g_Common.Light[ 0 ].Direction ;


	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 開始 )

	// 法線とライトの逆方向ベクトルとの内積を lLightLitParam.x にセット
	lLightLitParam.x = dot( lViewNrm, -lLightDir ) ;

	// ハーフベクトルの計算 norm( ( norm( 頂点位置から視点へのベクトル ) + ライトの方向 ) )
	lLightHalfVec = normalize( normalize( -lViewPosition.xyz ) - lLightDir ) ;

	// 法線とハーフベクトルの内積を lLightLitParam.y にセット
	lLightLitParam.y = dot( lLightHalfVec, lViewNrm ) ;

	// スペキュラ反射率を lLightLitParam.w にセット
	lLightLitParam.w = g_Common.Material.Power ;

	// ライト計算
	lLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;

	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 終了 )


	// カラー計算 ===========================================================( 開始 )

	// ディフューズライト蓄積値 += ディフューズ角度減衰計算結果 * マテリアルディフューズカラー * ライトのディフューズカラー + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの
	lTotalDiffuse += lLightLitDest.y * float4( g_Common.Light[ 0 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse + g_Common.Light[ 0 ].Ambient ;

	// スペキュラライト蓄積値 += スペキュラ角度減衰計算結果 * ライトのスペキュラカラー
	lTotalSpecular += lLightLitDest.z * float4( g_Common.Light[ 0 ].Specular, 0.0f ) ;

	// カラー計算 ===========================================================( 終了 )

	// ディレクショナルライトの処理 *****************************************************( 終了 )


	// ポイントライトの処理 *************************************************************( 開始 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( lViewPosition.xyz - g_Common.Light[ 1 ].Position ) ;


	// 距離減衰値計算 =======================================================( 開始 )

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = lViewPosition.xyz - g_Common.Light[ 1 ].Position ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( g_Common.Light[ 1 ].Attenuation0 + g_Common.Light[ 1 ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ 1 ].Attenuation2 * lLightDistancePow2 ) ;

	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ 1 ].RangePow2 ) ;

	// 距離減衰値計算 =======================================================( 終了 )


	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 開始 )

	// 法線とライトの逆方向ベクトルとの内積を lLightLitParam.x にセット
	lLightLitParam.x = dot( lViewNrm, -lLightDir ) ;

	// ハーフベクトルの計算 norm( ( norm( 頂点位置から視点へのベクトル ) + ライトの方向 ) )
	lLightHalfVec = normalize( normalize( -lViewPosition.xyz ) - lLightDir ) ;

	// 法線とハーフベクトルの内積を lLightLitParam.y にセット
	lLightLitParam.y = dot( lLightHalfVec, lViewNrm ) ;

	// スペキュラ反射率を lLightLitParam.w にセット
	lLightLitParam.w = g_Common.Material.Power ;

	// ライト計算
	lLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;

	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 =======( 終了 )


	// カラー計算 ===========================================================( 開始 )

	// ディフーズライト蓄積値 += 距離・スポットライト角度減衰値 * ( ディフーズ角度減衰計算結果 * マテリアルディフューズカラー * ライトのディフーズカラー + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの )
	lTotalDiffuse += lLightGen * ( lLightLitDest.y * float4( g_Common.Light[ 1 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse + g_Common.Light[ 1 ].Ambient ) ;

	// スペキュラライト蓄積値 += スペキュラ角度減衰計算結果 * 距離・スポットライト減衰 * ライトのスペキュラカラー
	lTotalSpecular += lLightGen * lLightLitDest.z * float4( g_Common.Light[ 1 ].Specular, 0.0f ) ;

	// カラー計算 ===========================================================( 終了 )

	// ポイントライトの処理 *************************************************************( 終了 )


	// ライトの処理 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ディフューズカラー = ディフューズライト蓄積値 + マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの
	VSOutput.Diffuse = lTotalDiffuse + g_Common.Material.Ambient_Emissive ;

	// ディフューズアルファはマテリアルのディフューズカラーのアルファをそのまま使う
	VSOutput.Diffuse.w = g_Common.Material.Diffuse.w ;

	// スペキュラカラー = スペキュラライト蓄積値 * マテリアルのスペキュラカラー
	VSOutput.Specular = lTotalSpecular * g_Common.Material.Specular ;


	// テクスチャ座標変換行列による変換を行った結果のテクスチャ座標をセット
	VSOutput.TexCoords0.x = dot( VSInput.TexCoords0, g_OtherMatrix.TextureMatrix[ 0 ][ 0 ] ) ;
	VSOutput.TexCoords0.y = dot( VSInput.TexCoords0, g_OtherMatrix.TextureMatrix[ 0 ][ 1 ] ) ;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 【D3D11】Direct3D 9 版(シェーダーモデル 2.0)は COLOR の出力が 0〜1 に切り詰められていた。同じ見た目にするため同じようにする
	VSOutput.Diffuse = saturate( VSOutput.Diffuse ) ;
	VSOutput.Specular = saturate( VSOutput.Specular ) ;

	// 出力パラメータを返す
	return VSOutput ;
}
