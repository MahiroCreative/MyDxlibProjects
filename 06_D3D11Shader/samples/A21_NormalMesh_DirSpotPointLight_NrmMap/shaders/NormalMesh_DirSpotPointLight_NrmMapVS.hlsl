// NormalMesh_DirSpotPointLight_NrmMapVS.fx の Direct3D 11 版(vs_4_0。法線マップ付き剛体メッシュ)
// reference/d3d9_original/A_base/21_NormalMesh_DirSpotPointLight_NrmMap から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibVS.hlsli"

// 頂点シェーダーの入力
// 【D3D11】DxLib の MV1 の頂点(法線マップ付き剛体メッシュ)の並び。使わない要素も省かず、この順番で書く(仕様書 5.3)
struct VS_INPUT
{
	float3 Position        : POSITION0     ;	// 座標( ローカル空間 )【D3D11】float3
	float3 Normal          : NORMAL0       ;	// 法線( ローカル空間 )
	float4 Diffuse         : COLOR0        ;	// ディフューズカラー
	float4 Specular        : COLOR1        ;	// スペキュラカラー
	float4 TexCoords0      : TEXCOORD0     ;	// テクスチャ座標( u, v, 1, 1 )
	float4 TexCoords1      : TEXCOORD1     ;	// サブテクスチャ座標【D3D11】使わなくても書く
	float3 Tan             : TANGENT0      ;	// 接線( ローカル空間 )
	float3 Bin             : BINORMAL0     ;	// 従法線( ローカル空間 )
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION     // 座標( プロジェクション空間 )
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VTan            : TEXCOORD2 ;    // 接線( ビュー空間 )
	float3 VBin            : TEXCOORD3 ;    // 従法線( ビュー空間 )
	float3 VNormal         : TEXCOORD4 ;    // 法線( ビュー空間 )
} ;


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;
	float3 lWorldNrm ;
	float3 lWorldTan ;
	float3 lWorldBin ;
	float3 lViewNrm ;
	float3 lViewTan ;
	float3 lViewBin ;


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


	// 法線をビュー空間の角度に変換 =====================================================( 開始 )

	// 従法線、接線、法線をビューベクトルに変換
	lWorldTan.x = dot( VSInput.Tan, g_Base.LocalWorldMatrix[ 0 ].xyz ) ;
	lWorldTan.y = dot( VSInput.Tan, g_Base.LocalWorldMatrix[ 1 ].xyz ) ;
	lWorldTan.z = dot( VSInput.Tan, g_Base.LocalWorldMatrix[ 2 ].xyz ) ;

	lWorldBin.x = dot( VSInput.Bin, g_Base.LocalWorldMatrix[ 0 ].xyz ) ;
	lWorldBin.y = dot( VSInput.Bin, g_Base.LocalWorldMatrix[ 1 ].xyz ) ;
	lWorldBin.z = dot( VSInput.Bin, g_Base.LocalWorldMatrix[ 2 ].xyz ) ;

	lWorldNrm.x = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 0 ].xyz ) ;
	lWorldNrm.y = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 1 ].xyz ) ;
	lWorldNrm.z = dot( VSInput.Normal, g_Base.LocalWorldMatrix[ 2 ].xyz ) ;

	lViewTan.x = dot( lWorldTan, g_Base.ViewMatrix[ 0 ].xyz ) ;
	lViewTan.y = dot( lWorldTan, g_Base.ViewMatrix[ 1 ].xyz ) ;
	lViewTan.z = dot( lWorldTan, g_Base.ViewMatrix[ 2 ].xyz ) ;

	lViewBin.x = dot( lWorldBin, g_Base.ViewMatrix[ 0 ].xyz ) ;
	lViewBin.y = dot( lWorldBin, g_Base.ViewMatrix[ 1 ].xyz ) ;
	lViewBin.z = dot( lWorldBin, g_Base.ViewMatrix[ 2 ].xyz ) ;

	lViewNrm.x = dot( lWorldNrm, g_Base.ViewMatrix[ 0 ].xyz ) ;
	lViewNrm.y = dot( lWorldNrm, g_Base.ViewMatrix[ 1 ].xyz ) ;
	lViewNrm.z = dot( lWorldNrm, g_Base.ViewMatrix[ 2 ].xyz ) ;

	// 法線をビュー空間の角度に変換 =====================================================( 終了 )


	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// テクスチャ座標変換行列による変換を行った結果のテクスチャ座標をセット
	VSOutput.TexCoords0.x = dot( VSInput.TexCoords0, g_OtherMatrix.TextureMatrix[ 0 ][ 0 ] ) ;
	VSOutput.TexCoords0.y = dot( VSInput.TexCoords0, g_OtherMatrix.TextureMatrix[ 0 ][ 1 ] ) ;

	// 頂点座標を保存
	VSOutput.VPosition = lViewPosition.xyz ;

	// 接線を保存
	VSOutput.VTan = lViewTan ;

	// 従法線を保存
	VSOutput.VBin = lViewBin ;

	// 法線を保存
	VSOutput.VNormal = lViewNrm ;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return VSOutput ;
}

