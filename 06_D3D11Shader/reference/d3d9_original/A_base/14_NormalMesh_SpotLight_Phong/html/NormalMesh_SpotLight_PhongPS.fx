// ピクセルシェーダーの入力
struct PS_INPUT
{
	float2 TexCoords0      : TEXCOORD0 ;    // テクスチャ座標
	float3 VPosition       : TEXCOORD1 ;    // 座標( ビュー空間 )
	float3 VNormal         : TEXCOORD2 ;    // 法線( ビュー空間 )
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : COLOR0 ;
} ;

// マテリアルパラメータ
struct MATERIAL
{
	float4 Diffuse ;      // ディフューズカラー
	float4 Specular ;     // スペキュラカラー
	float4 Power ;        // スペキュラの強さ
} ;

// ライトパラメータ
struct LIGHT
{
	float4 Position ;               // 座標( ビュー空間 )
	float3 Direction ;              // 方向( ビュー空間 )
	float4 Diffuse ;                // ディフューズカラー
	float4 Specular ;               // スペキュラカラー
	float4 Ambient ;                // アンビエントカラーとマテリアルのアンビエントカラーを乗算したもの
	float4 Range_FallOff_AT0_AT1 ;  // x:有効距離  y:スポットライト用FallOff  z:距離による減衰処理用パラメータ０  w:距離による減衰処理用パラメータ１
	float4 AT2_SpotP0_SpotP1 ;      // x:距離による減衰処理用パラメータ２  y:スポットライト用パラメータ０( cos( Phi / 2.0f ) )  z:スポットライト用パラメータ１( 1.0f / ( cos( Theta / 2.0f ) - cos( Phi / 2.0f ) ) )
} ;


// C++ 側で設定するテクスチャや定数の定義
sampler  DiffuseMapTexture             : register( s0 ) ;		// ディフューズマップテクスチャ
float4   cfAmbient_Emissive            : register( c1 ) ;		// エミッシブカラー + マテリアルアンビエントカラー * グローバルアンビエントカラー
MATERIAL cfMaterial                    : register( c2 ) ;		// マテリアルパラメータ
float4   cfFactorColor                 : register( c5 ) ;		// 不透明度等
LIGHT    cfLight[ 1 ]                  : register( c32 ) ;		// ライトパラメータ


// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 SpecularColor ;
	float3 Normal ;
	float DiffuseAngleGen ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float3 V_to_Eye ;
	float3 TempF3 ;
	float Temp ;
	float3 lLightTemp ;
	float lLightDistancePow2 ;
	float lLightGen ;
	float3 lLightDir ;
	float lLightDirectionCosA ;


	// 法線の準備
	Normal = normalize( PSInput.VNormal ) ;

	// 頂点座標から視点へのベクトルを正規化
	V_to_Eye = normalize( -PSInput.VPosition ) ;

	// ディフューズカラーとスペキュラカラーの蓄積値を初期化
	TotalDiffuse  = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	TotalSpecular = float4( 0.0f, 0.0f, 0.0f, 0.0f ) ;


	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// ライト方向ベクトルの計算
	lLightDir = normalize( PSInput.VPosition.xyz - cfLight[ 0 ].Position.xyz ) ;


	// 距離・スポットライト減衰値計算 =======================================( 開始 )

	// 距離減衰計算 ------------------

	// 頂点とライト位置との距離の二乗を求める
	lLightTemp = PSInput.VPosition.xyz - cfLight[ 0 ].Position.xyz ;
	lLightDistancePow2 = dot( lLightTemp, lLightTemp ) ;

	// 減衰率の計算 lLightGen = 1 / ( 減衰値0 + 減衰値1 * 距離 + 減衰値2 * ( 距離 * 距離 ) )
	lLightGen = 1.0f / ( cfLight[ 0 ].Range_FallOff_AT0_AT1.z + cfLight[ 0 ].Range_FallOff_AT0_AT1.w * sqrt( lLightDistancePow2 ) + cfLight[ 0 ].AT2_SpotP0_SpotP1.x * lLightDistancePow2 ) ;

	// --------------------------------


	// スポットライト減衰計算 --------

	// ライト方向ベクトルとライト位置から頂点位置へのベクトルの内積( 即ち Cos a )を計算 
	lLightDirectionCosA = dot( lLightDir, cfLight[ 0 ].Direction ) ;

	// スポットライト減衰計算  pow( falloff, ( ( Cos a - Cos f ) / ( Cos q - Cos f ) ) )
	lLightGen *= saturate( pow( abs( max( lLightDirectionCosA - cfLight[ 0 ].AT2_SpotP0_SpotP1.y, 0.0f ) * cfLight[ 0 ].AT2_SpotP0_SpotP1.z ), cfLight[ 0 ].Range_FallOff_AT0_AT1.y ) ) ;

	// --------------------------------


	// 有効距離外だったら減衰率を最大にする処理
	lLightGen *= step( lLightDistancePow2, cfLight[ 0 ].Range_FallOff_AT0_AT1.x ) ;

	// 距離・スポットライト減衰値計算 =======================================( 終了 )


	// ディフューズ色計算

	// ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, -lLightDir ) ) ;

	// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * マテリアルディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
	TotalDiffuse += ( cfLight[ 0 ].Diffuse * cfMaterial.Diffuse * DiffuseAngleGen + cfLight[ 0 ].Ambient ) * lLightGen ;


	// スペキュラカラー計算

	// ハーフベクトルの計算
	TempF3 = normalize( V_to_Eye - lLightDir ) ;

	// Temp = pow( max( 0.0f, N * H ), cfMaterial.Power.x )
	Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), cfMaterial.Power.x ) ;

	// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
	TotalSpecular += Temp * lLightGen.x * cfLight[ 0 ].Specular ;

	// スポットライトの処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )



	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

	// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
	TotalDiffuse += cfAmbient_Emissive ;

	// SpecularColor = ライトのスペキュラカラー蓄積値 * マテリアルのスペキュラカラー
	SpecularColor = TotalSpecular * cfMaterial.Specular ;

	// 出力カラー = TotalDiffuse * テクスチャカラー + SpecularColor
	TextureDiffuseColor = tex2D( DiffuseMapTexture, PSInput.TexCoords0.xy ) ;
	PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;

	// アルファ値 = テクスチャアルファ * マテリアルのディフューズアルファ * 不透明度
	PSOutput.Color0.a = TextureDiffuseColor.a * cfMaterial.Diffuse.a * cfFactorColor.a ;

	// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


	// 出力パラメータを返す
	return PSOutput ;
}
