// VertexShaderTest のピクセルシェーダー(Direct3D 11 版 / ps_4_0)
// Direct3D 9 版からの変更点には【D3D11】と書いています。

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ順番で書く。Direct3D 11 は名前ではなく順番で値を結び付ける。
struct PS_INPUT
{
    float4 ProjectionPosition   : SV_POSITION ; // 【D3D11】頂点シェーダーの出力の先頭にあるので、使わなくても書く
    float4 DiffuseColor         : COLOR0 ;      // ディフューズカラー
    float2 TextureCoord0        : TEXCOORD0 ;   // テクスチャ座標
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 DrawColor            : SV_TARGET0 ;  // 描画カラー【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定する定数の定義

// 描画するテクスチャ
// 【D3D11】sampler 1 個の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する。番号は SetUseTextureToShader の番号
Texture2D    DiffuseMapTexture  : register( t0 ) ;
SamplerState DiffuseMapSampler  : register( s0 ) ;

// 【D3D11】C++ 側で設定する定数(CreateShaderConstantBuffer で作り、スロット 4 にセットしたもの)
cbuffer VertexShaderTestPSParam : register( b4 )
{
    float4 cfMultiplyColor ;    // 描画カラーに乗算する値
} ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    // 【D3D11】tex2D( サンプラー, 座標 ) → テクスチャ.Sample( サンプラー, 座標 )
    lTextureColor = DiffuseMapTexture.Sample( DiffuseMapSampler, PSInput.TextureCoord0 ) ;

    // 出力する色はテクスチャの色と C++ で設定した値とディフューズカラーを乗算したもの
    PSOutput.DrawColor = lTextureColor * cfMultiplyColor * PSInput.DiffuseColor ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
