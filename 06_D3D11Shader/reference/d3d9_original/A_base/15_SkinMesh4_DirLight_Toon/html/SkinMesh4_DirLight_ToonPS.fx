// ピクセルシェーダーの入力
struct PS_INPUT
{
	float4 Diffuse         : COLOR0 ;       // ディフューズカラー
	float4 Specular        : COLOR1 ;       // スペキュラカラー
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float2 ToonCoords0     : TEXCOORD1 ;    // トゥーンテクスチャ座標
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : COLOR0 ;
} ;



// C++ 側で設定するテクスチャの定義
sampler  DiffuseMapTexture             : register( s0 ) ;		// ディフューズマップテクスチャ
sampler  ToonTexture                   : register( s1 ) ;		// トゥーンテクスチャ
float4   cfFactorColor                 : register( c5 ) ;		// 不透明度等



// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 ToonColor ;

	// テクスチャカラーの取得
	TextureDiffuseColor = tex2D( DiffuseMapTexture, PSInput.TexCoords0 ) ;

	// トゥーンテクスチャカラーを取得
	ToonColor = tex2D( ToonTexture, PSInput.ToonCoords0 ) ;

	// 出力 = saturate( ディフューズカラー * トゥーンテクスチャカラー + スペキュラカラー ) * テクスチャカラー
	PSOutput.Color0.rgb = saturate( PSInput.Diffuse.rgb * ToonColor.rgb + PSInput.Specular.rgb ) * TextureDiffuseColor.rgb ;

	// アルファ値 = ディフューズアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * PSInput.Diffuse.a * cfFactorColor.a ;


	// 出力パラメータを返す
	return PSOutput ;
}
