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
    float4 Output             : COLOR0 ;
} ;


// C++ 側で設定する定数の定義

// 描画するテクスチャ
sampler DiffuseMapTexture : register( s0 ) ;

// テクスチャの色に乗算する値
float4 cfMultiplyColor : register( c0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    lTextureColor  = tex2D( DiffuseMapTexture, PSInput.TextureCoord0 ) ; 

    // 出力する色はテクスチャの色と定数を乗算したもの
    PSOutput.Output = lTextureColor * cfMultiplyColor ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
