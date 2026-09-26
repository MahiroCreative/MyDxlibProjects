// NormalMesh_PointLight6_PhongPS.fx の Direct3D 11 版(ps_4_0)
// reference/d3d9_original/A_base/13_NormalMesh_PointLight6_Phong から移植。Direct3D 9 版からの変更点には【D3D11】と書いています。
// DxLib が渡す定数(g_Common / g_Base など)の意味は spec/dxlib_d3d11_shader_spec.md の 8 章。

// 【D3D11】DxLib が渡す定数の宣言(Direct3D 9 版の register( c○ ) の定数の代わり)
#include "DxLibPS.hlsli"

// ピクセルシェーダーの入力
// 【D3D11】頂点シェーダーの出力(VS_OUTPUT)と同じ並びにする(名前ではなく順番で結び付くため)
struct PS_INPUT
{
	float4 Position        : SV_POSITION ;	// 【D3D11】POSITION → SV_POSITION     // 座標( プロジェクション空間 )
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VNormal         : TEXCOORD2 ;    // 法線( ビュー空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : SV_TARGET0 ;	// 【D3D11】COLOR0 → SV_TARGET0
} ;


// C++ 側で設定するテクスチャや定数の定義
// 【D3D11】sampler の代わりに、テクスチャ(t0)とサンプラー(s0)を別々に宣言する
Texture2D    DiffuseMapTexture : register( t0 ) ;		// ディフューズマップテクスチャ
SamplerState DiffuseMapTextureSampler : register( s0 ) ;


// ポイントライトの計算をする関数
inline void CalcPointLight(
	const in    PS_INPUT PSInput,
	const in    int      LightNo,
	const in    float3   Normal,
	const in    float3   V_to_Eye,
	      inout float4   TotalDiffuse,
	      inout float4   TotalSpecular
)
{
	float  DiffuseAngleGen ;
	float3 TempF3 ;
	float  Temp ;
	float3 lLightTemp ;
	float  lLightDistancePow2 ;
	float  lLightGen ;
	float3 lLightDir ;

	// ライト方向ベクトルの計算
	lLightDir = normalize( PSInput.VPosition.xyz - g_Common.Light[ LightNo ].Position ) ;


	// 距離減衰値計算 =======================================================( 開始 )

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = PSInput.VPosition.xyz - g_Common.Light[ LightNo ].Position ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( g_Common.Light[ LightNo ].Attenuation0 + g_Common.Light[ LightNo ].Attenuation1 * sqrt( lLightDistancePow2 ) + g_Common.Light[ LightNo ].Attenuation2 * lLightDistancePow2 ) ;

	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, g_Common.Light[ LightNo ].RangePow2 ) ;

	// 距離減衰値計算 =======================================================( 終了 )


	// ディフューズ色計算

	// ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * マテリアルディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
	TotalDiffuse += ( float4( g_Common.Light[ LightNo ].Diffuse, 0.0f ) * g_Common.Material.Diffuse * DiffuseAngleGen + g_Common.Light[ LightNo ].Ambient ) * lLightGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), g_Common.Material.Power )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), g_Common.Material.Power ) ;

	// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	TotalSpecular += Temp * lLightGen.x * float4( g_Common.Light[ LightNo ].Specular, 0.0f ) ;
}


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 SpecularColor ;
	float3 Normal ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float3 V_to_Eye ;


	// 法線の準備
	Normal = normalize( PSInput.VNormal ) ;

	// 頂点座標から視点へのベクトルを正規化
	V_to_Eye = normalize( -PSInput.VPosition ) ;

	// ディフューズカラーとスペキュラカラーの蓄積値を初期化
	TotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	TotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;


	// ポイントライト０～５の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	CalcPointLight( PSInput, 0, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;
	CalcPointLight( PSInput, 1, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;
	CalcPointLight( PSInput, 2, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;
	CalcPointLight( PSInput, 3, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;
	CalcPointLight( PSInput, 4, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;
	CalcPointLight( PSInput, 5, Normal, V_to_Eye, TotalDiffuse, TotalSpecular ) ;

	// ポイントライト０～５の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
	TotalDiffuse += g_Common.Material.Ambient_Emissive ;

	// SpecularColor = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	SpecularColor = TotalSpecular * g_Common.Material.Specular ;

	// 出力カラー = TotalDiffuse * テクスチャカラー + SpecularColor
	TextureDiffuseColor = DiffuseMapTexture.Sample( DiffuseMapTextureSampler, PSInput.TexCoords0.xy ) ;
	PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;

	// アルファ値 = テクスチャアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * g_Common.Material.Diffuse.a * g_Base.FactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}
