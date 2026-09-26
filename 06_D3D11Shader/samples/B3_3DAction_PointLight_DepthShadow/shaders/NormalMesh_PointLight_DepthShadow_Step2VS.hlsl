// NormalMesh_PointLight_DepthShadow_Step2VS.fx の Direct3D 11 版(vs_4_0。剛体メッシュ)
// reference/d3d9_original/B_applied/3DAction_PointLight_DepthShadow から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
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


// C++ 側で設定する定数の定義

// 【D3D11】C++ から渡す値(Direct3D 9 版の register( c○ ) の定数の代わり。C++ で CreateShaderConstantBuffer で作り、スロット 4 にセットする)
// packoffset( c○ ) は定数バッファの先頭から何番目の float4 か(Direct3D 9 版の c43 が c0)
cbuffer UserParam : register( b4 )
{
	matrix cfDepthViewMatrix0 : packoffset( c0 ) ;	// 深度バッファ０レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix0 : packoffset( c4 ) ;	// 深度バッファ０レンダリング時のビュー　　→　射影行列
	matrix cfDepthViewMatrix1 : packoffset( c8 ) ;	// 深度バッファ１レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix1 : packoffset( c12 ) ;	// 深度バッファ１レンダリング時のビュー　　→　射影行列
	matrix cfDepthViewMatrix2 : packoffset( c16 ) ;	// 深度バッファ２レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix2 : packoffset( c20 ) ;	// 深度バッファ２レンダリング時のビュー　　→　射影行列
	matrix cfDepthViewMatrix3 : packoffset( c24 ) ;	// 深度バッファ３レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix3 : packoffset( c28 ) ;	// 深度バッファ３レンダリング時のビュー　　→　射影行列
	matrix cfDepthViewMatrix4 : packoffset( c32 ) ;	// 深度バッファ４レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix4 : packoffset( c36 ) ;	// 深度バッファ４レンダリング時のビュー　　→　射影行列
	matrix cfDepthViewMatrix5 : packoffset( c40 ) ;	// 深度バッファ５レンダリング時のワールド　→　ビュー行列
	matrix cfDepthProjectionMatrix5 : packoffset( c44 ) ;	// 深度バッファ５レンダリング時のビュー　　→　射影行列
} ;


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;
	float4 lDViewPosition ;
	float3 lWorldNrm ;
	float3 lViewNrm ;
	float3 lLightHalfVec ;
	float4 lLightLitParam ;
	float4 lLightLitDest ;
	float3 lLightDir ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightGen ;

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

	// 法線をビュー空間の角度に変換 =========================================( 開始 )

	// ローカルベクトルをワールドベクトルに変換
	lWorldNrm.x = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 0 ].xyz ) ;
	lWorldNrm.y = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 1 ].xyz ) ;
	lWorldNrm.z = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 2 ].xyz ) ;

	// ワールドベクトルをビューベクトルに変換
	lViewNrm.x = dot( lWorldNrm, g_Base.ViewMatrix[ 0 ].xyz ) ;
	lViewNrm.y = dot( lWorldNrm, g_Base.ViewMatrix[ 1 ].xyz ) ;
	lViewNrm.z = dot( lWorldNrm, g_Base.ViewMatrix[ 2 ].xyz ) ;

	// 法線をビュー空間の角度に変換 =========================================( 終了 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( lViewPosition.xyz - g_Common.Light[ 0 ].Position ) ;


	// 距離減衰値計算 ===================================================================( 開始 )

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = lViewPosition.xyz - g_Common.Light[ 0 ].Position ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( g_Common.Light[ 0 ].Attenuation0 + g_Common.Light[ 0 ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ 0 ].Attenuation2 * lLightDistancePow2 ) ;

	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ 0 ].RangePow2 ) ;

	// 距離減衰値計算 ===================================================================( 終了 )


	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 ===================( 開始 )

	// 法線とライトの逆方向ベクトルとの内積を lLightLitParam.x にセット
	lLightLitParam.x = dot( lViewNrm, -lLightDir ) ;

	// ハーフベクトルの計算 norm( ( norm( 頂点位置から視点へのベクトル ) + ライトの方向 ) )
	lLightHalfVec = normalize( normalize( -lViewPosition.xyz ) - lLightDir ) ;

	// 法線とハーフベクトルの内積を lLightLitParam.y にセット
	lLightLitParam.y = dot( lLightHalfVec, lViewNrm ) ;

	// スペキュラ反射率を lLightLitParam.w にセット
	lLightLitParam.w = g_Common.Material.Power ;

	// ライトパラメータ計算
	lLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;

	// ライトディフューズカラーとライトスペキュラカラーの角度減衰計算 ===================( 終了 )

	// ライトの処理 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ディフューズカラー =
	//            距離減衰値 *
	//            ( ディフューズ角度減衰計算結果 *
	//              ライトのディフューズカラー *
	//              マテリアルのディフューズカラー +
	//              ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの ) +
	//            マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの
	VSOutput.Diffuse = lLightGen * ( lLightLitDest.y * float4( g_Common.Light[ 0 ].Diffuse, 0.0f ) * g_Common.Material.Diffuse + g_Common.Light[ 0 ].Ambient ) + g_Common.Material.Ambient_Emissive ;

	// ディフューズアルファはマテリアルのディフューズカラーのアルファをそのまま使う
	VSOutput.Diffuse.w = g_Common.Material.Diffuse.w ;

	// スペキュラカラー = 距離減衰値 * スペキュラ角度減衰計算結果 * ライトのスペキュラカラー * マテリアルのスペキュラカラー
	VSOutput.Specular = lLightGen * lLightLitDest.z * float4( g_Common.Light[ 0 ].Specular, 0.0f ) * g_Common.Material.Specular ;


	// テクスチャ座標のセット
	VSOutput.TexCoords0 = VSInput.TexCoords0;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 深度バッファレンダリング時のカメラ設定での射影行列を算出 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	lDViewPosition = mul( cfDepthViewMatrix0, lWorldPosition ) ;
	VSOutput.DPPosition0 = mul( cfDepthProjectionMatrix0, lDViewPosition ) ;

	lDViewPosition = mul( cfDepthViewMatrix1, lWorldPosition ) ;
	VSOutput.DPPosition1 = mul( cfDepthProjectionMatrix1, lDViewPosition ) ;

	lDViewPosition = mul( cfDepthViewMatrix2, lWorldPosition ) ;
	VSOutput.DPPosition2 = mul( cfDepthProjectionMatrix2, lDViewPosition ) ;

	lDViewPosition = mul( cfDepthViewMatrix3, lWorldPosition ) ;
	VSOutput.DPPosition3 = mul( cfDepthProjectionMatrix3, lDViewPosition ) ;

	lDViewPosition = mul( cfDepthViewMatrix4, lWorldPosition ) ;
	VSOutput.DPPosition4 = mul( cfDepthProjectionMatrix4, lDViewPosition ) ;

	lDViewPosition = mul( cfDepthViewMatrix5, lWorldPosition ) ;
	VSOutput.DPPosition5 = mul( cfDepthProjectionMatrix5, lDViewPosition ) ;

	// 深度バッファレンダリング時のカメラ設定での射影行列を算出 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return VSOutput ;
}

