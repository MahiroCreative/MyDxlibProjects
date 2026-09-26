// VertexShaderTest の頂点シェーダー(C1 と同じもの)(Direct3D 11 版 / vs_4_0)
// Direct3D 9 版からの変更点には【D3D11】と書いています。

// 【D3D11】DxLib が渡す定数(ビュー行列・射影行列など)の宣言
#include "DxLibVS.hlsli"

// 頂点シェーダーの入力
// 【D3D11】DrawPolygon3DToShader の頂点(VERTEX3DSHADER)の全要素を、この順番で書く。使わない要素も省かない。
// 省いたり並びを変えたりすると、エラーも出ずに何も描かれない。
struct VS_INPUT
{
    float3 Position             : POSITION0 ;   // 座標( VERTEX3DSHADER構造体の pos の値 )【D3D11】float3
    float4 SubPosition          : POSITION1 ;   // 補助座標( spos )
    float3 Normal               : NORMAL0 ;     // 法線( norm )
    float3 Tangent              : TANGENT0 ;    // 接線( tan )
    float3 Binormal             : BINORMAL0 ;   // 従法線( binorm )
    float4 DiffuseColor         : COLOR0 ;      // ディフューズカラー( dif )
    float4 SpecularColor        : COLOR1 ;      // スペキュラカラー( spc )
    float2 TextureCoord0        : TEXCOORD0 ;   // テクスチャ座標０( u, v )
    float2 TextureCoord1        : TEXCOORD1 ;   // テクスチャ座標１( su, sv )
} ;

// 頂点シェーダーの出力(ピクセルシェーダーの入力と同じ順番にする)
struct VS_OUTPUT
{
    float4 ProjectionPosition   : SV_POSITION ; // 座標( 射影空間 )【D3D11】POSITION → SV_POSITION
    float4 DiffuseColor         : COLOR0 ;      // ディフューズカラー
    float2 TextureCoord0        : TEXCOORD0 ;   // テクスチャ座標
} ;

// 【D3D11】C++ 側で設定する定数(CreateShaderConstantBuffer で作り、スロット 4 にセットしたもの)
cbuffer VertexShaderTestParam : register( b4 )
{
    float4 cfAddPosition ;      // 頂点座標に加算する値
} ;


// main関数
VS_OUTPUT main( VS_INPUT VSInput )
{
    VS_OUTPUT VSOutput ;
    float4 lWorldPosition ;
    float4 lViewPosition ;

    // 入力の頂点座標にＣ＋＋プログラム側で設定した頂点座標を加算する
    // 【D3D11】入力は float3 なので w = 1 を付けて float4 にする
    lWorldPosition = float4( VSInput.Position, 1.0f ) + cfAddPosition ;

    // 頂点座標をビュー空間の座標に変換する
    // 【D3D11】g_Base.ViewMatrix はビュー行列を転置した 3 行。各行との内積が変換後の x, y, z
    lViewPosition.x = dot( lWorldPosition, g_Base.ViewMatrix[ 0 ] ) ;
    lViewPosition.y = dot( lWorldPosition, g_Base.ViewMatrix[ 1 ] ) ;
    lViewPosition.z = dot( lWorldPosition, g_Base.ViewMatrix[ 2 ] ) ;
    lViewPosition.w = 1.0f ;

    // ビュー空間の座標を射影空間の座標に変換する
    VSOutput.ProjectionPosition.x = dot( lViewPosition, g_Base.ProjectionMatrix[ 0 ] ) ;
    VSOutput.ProjectionPosition.y = dot( lViewPosition, g_Base.ProjectionMatrix[ 1 ] ) ;
    VSOutput.ProjectionPosition.z = dot( lViewPosition, g_Base.ProjectionMatrix[ 2 ] ) ;
    VSOutput.ProjectionPosition.w = dot( lViewPosition, g_Base.ProjectionMatrix[ 3 ] ) ;

    // テクスチャ座標はそのまま代入
    VSOutput.TextureCoord0 = VSInput.TextureCoord0 ;

    // 頂点カラーはそのまま代入
    VSOutput.DiffuseColor = VSInput.DiffuseColor ;

    // 関数の戻り値がピクセルシェーダーに渡される
    return VSOutput ;
}
