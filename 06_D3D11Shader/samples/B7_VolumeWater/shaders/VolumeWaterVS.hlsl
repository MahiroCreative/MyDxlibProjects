// VolumeWaterVS.fx の Direct3D 11 版(vs_4_0。DrawPolygon3DToShader 用)
// reference/d3d9_original/B_applied/VolumeWater から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.html の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibVS.hlsli"

// 頂点シェーダーの入力
// 【D3D11】DrawPolygon3DToShader の頂点(VERTEX3DSHADER)の 9 要素を、使わない要素も省かず、この順番で書く(仕様書 5.1)
struct VS_INPUT
{
	float3 Position                 : POSITION0 ;	// 座標( pos )【D3D11】float3
	float4 SubPosition              : POSITION1 ;	// 補助座標( spos )
	float3 Normal                   : NORMAL0   ;	// 法線( norm )
	float3 Tangent                  : TANGENT0  ;	// 接線( tan )【D3D11】使わなくても書く
	float3 Binormal                 : BINORMAL0 ;	// 従法線( binorm )【D3D11】使わなくても書く
	float4 Diffuse                  : COLOR0    ;	// ディフューズカラー( dif )
	float4 Specular                 : COLOR1    ;	// スペキュラカラー( spc )
	float2 TexCoords0               : TEXCOORD0 ;	// テクスチャ座標( u, v )【D3D11】float2
	float2 TexCoords1               : TEXCOORD1 ;	// テクスチャ座標( su, sv )【D3D11】float2
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 ProjectionPosition       : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION	// 座標( 射影空間 )
	float4 ViewPosition             : TEXCOORD0 ;	// 座標( ビュー空間 )
	float4 ProjectionPositionSub    : TEXCOORD1 ;	// 座標( 射影空間 )ピクセルシェーダーで参照する為の物
	float4 Diffuse                  : COLOR0 ;		// ディフューズカラー
} ;


// C++ 側で設定する定数の定義


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 lWorldPosition ;


	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ローカル座標をワールド座標に変換
	lWorldPosition.x = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 0 ] ) ;
	lWorldPosition.y = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 1 ] ) ;
	lWorldPosition.z = dot( float4( VSInput.Position, 1.0f ), g_Base.LocalWorldMatrix[ 2 ] ) ;
	lWorldPosition.w = 1.0f ;

	// ワールド座標をビュー座標に変換
	VSOutput.ViewPosition.x = dot( lWorldPosition, g_Base.ViewMatrix[ 0 ] ) ;
	VSOutput.ViewPosition.y = dot( lWorldPosition, g_Base.ViewMatrix[ 1 ] ) ;
	VSOutput.ViewPosition.z = dot( lWorldPosition, g_Base.ViewMatrix[ 2 ] ) ;
	VSOutput.ViewPosition.w = 1.0f ;

	// ビュー座標を射影座標に変換
	VSOutput.ProjectionPosition.x = dot( VSOutput.ViewPosition, g_Base.ProjectionMatrix[ 0 ] ) ;
	VSOutput.ProjectionPosition.y = dot( VSOutput.ViewPosition, g_Base.ProjectionMatrix[ 1 ] ) ;
	VSOutput.ProjectionPosition.z = dot( VSOutput.ViewPosition, g_Base.ProjectionMatrix[ 2 ] ) ;
	VSOutput.ProjectionPosition.w = dot( VSOutput.ViewPosition, g_Base.ProjectionMatrix[ 3 ] ) ;
	
	// 頂点座標変換 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// ピクセルシェーダーで使用するための射影座標をセット
	VSOutput.ProjectionPositionSub = VSOutput.ProjectionPosition ;

	// ディフューズカラーはそのまま渡す
	VSOutput.Diffuse = VSInput.Diffuse ;


	// 出力パラメータを返す
	// 【D3D11】Direct3D 9 版(シェーダーモデル 2.0)は COLOR の出力が 0〜1 に切り詰められていた。同じ見た目にするため同じようにする
	VSOutput.Diffuse = saturate( VSOutput.Diffuse ) ;

	return VSOutput ;
}
