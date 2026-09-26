// PixelShaderTest のピクセルシェーダー(C2 と同じもの)(Direct3D 11 版 / ps_4_0)。2D の描画(DrawPolygon2DToShader)用。
// Direct3D 9 版からの変更点には【D3D11】と書いています。

// ピクセルシェーダーの入力
// 【D3D11】2D の描画では DxLib の頂点シェーダー(VS_Shader2D)が必ず使われ、その出力がこの順番で渡される。
// Direct3D 11 は名前ではなく順番で値を結び付けるので、先頭の SV_POSITION も含めて、この順番で全部書く。
struct PS_INPUT
{
    float4 Position           : SV_POSITION ;   // 【D3D11】画面上の座標(Direct3D 9 版には無い)
    float4 DiffuseColor       : COLOR0 ;
    float4 SpecularColor      : COLOR1 ;
    float2 TextureCoord0      : TEXCOORD0 ;
    float2 TextureCoord1      : TEXCOORD1 ;
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 Output             : SV_TARGET0 ;    // 【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定する定数の定義

// 描画するテクスチャ
// 【D3D11】sampler 1 個の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する。番号は SetUseTextureToShader の番号
Texture2D    DiffuseMapTexture : register( t0 ) ;
SamplerState DiffuseMapSampler : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    // 【D3D11】tex2D( サンプラー, 座標 ) → テクスチャ.Sample( サンプラー, 座標 )
    lTextureColor  = DiffuseMapTexture.Sample( DiffuseMapSampler, PSInput.TextureCoord0 ) ;

    // 出力する色は青成分と赤成分を逆転したもの
    PSOutput.Output.r = lTextureColor.b ;
    PSOutput.Output.g = lTextureColor.g ;
    PSOutput.Output.b = lTextureColor.r ;
    PSOutput.Output.a = lTextureColor.a ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
