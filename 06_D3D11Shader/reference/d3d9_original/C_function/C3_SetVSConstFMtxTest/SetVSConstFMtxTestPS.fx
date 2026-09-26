// ピクセルシェーダーの入力
struct PS_INPUT
{
    float4 DiffuseColor         : COLOR0 ;      // ディフューズカラー
    float2 TextureCoord0        : TEXCOORD0 ;   // テクスチャ座標
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
    float4 DrawColor            : COLOR0 ;    // 描画カラー
} ;


// C++ 側で設定する定数の定義

// 描画するテクスチャ
sampler  DiffuseMapTexture      : register( s0 ) ;


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
    PS_OUTPUT PSOutput ;
    float4 lTextureColor ;

    // テクスチャーの色を取得
    lTextureColor = tex2D( DiffuseMapTexture, PSInput.TextureCoord0 ) ;

    // 出力する色はテクスチャの色とディフューズカラーを乗算したもの
    PSOutput.DrawColor = lTextureColor * PSInput.DiffuseColor ;

    // 関数の戻り値がラスタライザに渡される
    return PSOutput ;
}
