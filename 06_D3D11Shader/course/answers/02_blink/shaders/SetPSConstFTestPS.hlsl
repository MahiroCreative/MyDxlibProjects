// SetPSConstFTest のピクセルシェーダー(Direct3D 11 版 / ps_4_0)。2D の描画(DrawPolygon2DToShader)用。
// Direct3D 9 版からの変更点には【D3D11】と書いています。

// ピクセルシェーダーの入力
// 【D3D11】2D の描画では DxLib の頂点シェーダー(VS_Shader2D)が必ず使われ、その出力がこの順番で渡される。
// 先頭の SV_POSITION も含めて、この順番で全部書く。
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
// 【D3D11】sampler 1 個の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;
SamplerState DiffuseMapSampler : register( s0 ) ;

// 【D3D11】C++ 側で設定する定数(CreateShaderConstantBuffer で作り、スロット 4 にセットしたもの)
cbuffer SetPSConstFTestParam : register( b4 )
{
    float4 cfTime ;             // 【課題】x: 経過時間(秒)。C++ の f4.x
} ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    // 【D3D11】tex2D( サンプラー, 座標 ) → テクスチャ.Sample( サンプラー, 座標 )
    lTextureColor  = DiffuseMapTexture.Sample( DiffuseMapSampler, PSInput.TextureCoord0 ) ;

    // 【課題 2-1】経過時間で明るさを 0〜1 の間で波のように変える。sin は -1〜1 なので、0.5 倍して 0.5 足す
    float lBright = sin( cfTime.x * 3.0f ) * 0.5f + 0.5f ;
    PSOutput.Output = float4( lTextureColor.rgb * lBright, lTextureColor.a ) ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
