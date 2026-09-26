// NormalMesh_DepthShadow_Step1VS.fx の Direct3D 11 版(vs_4_0。剛体メッシュ)
// reference/d3d9_original/B_applied/3DAction_PointLight_DepthShadow から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibVS.hlsli"

// 頂点シェーダーの入力
// 【D3D11】DxLib の MV1 の頂点(剛体メッシュ)の並び。使わない要素も省かず、この順番で書く(仕様書 5.3)
// 【D3D11】原本の入力には BLENDINDICES0 / BLENDWEIGHT0 があった(スキンメッシュ用からの写し残りで、使っていない)。
// 剛体メッシュの頂点には無いので外した。Direct3D 9 は無い入力に既定の値を入れて描いたが、Direct3D 11 は入力が頂点データと合わないと何も描かない
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
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION         // 座標( 射影空間 )
	float4 PPosition       : TEXCOORD0 ;        // 座標( 射影空間 )
} ;


// C++ 側で設定する定数の定義


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;
	float4 lViewPosition ;


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


	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// 射影座標をテクスチャ座標として出力する
	VSOutput.PPosition = VSOutput.Position ;

	// 出力パラメータセット ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return VSOutput ;
}
