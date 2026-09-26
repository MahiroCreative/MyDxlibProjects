// ピクセルシェーダーの入力
struct PS_INPUT
{
    float4 DiffuseColor       : COLOR0 ;
    float4 SpecularColor      : COLOR1 ;
    float2 TextureCoord0      : TEXCOORD0 ;
    float2 TextureCoord1      : TEXCOORD1 ;
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 Output0            : COLOR0 ;
    float4 Output1            : COLOR1 ;
} ;


// C++ 側で設定する定数の定義

// 描画するテクスチャ
sampler DiffuseMapTexture : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    lTextureColor  = tex2D( DiffuseMapTexture, PSInput.TextureCoord0 ) ; 

    // 出力先０には赤成分だけ出力
    PSOutput.Output0.r = lTextureColor.r ;
    PSOutput.Output0.g = 0.0f ;
    PSOutput.Output0.b = 0.0f ;
    PSOutput.Output0.a = 1.0f ;

    // 出力先１には緑成分だけ出力
    PSOutput.Output1.r = 0.0f ;
    PSOutput.Output1.g = lTextureColor.g ;
    PSOutput.Output1.b = 0.0f ;
    PSOutput.Output1.a = 1.0f ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
