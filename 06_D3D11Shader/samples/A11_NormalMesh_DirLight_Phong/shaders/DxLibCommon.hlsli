// DxLib 3.24f(Direct3D 11)が自作シェーダーに渡す定数の、頂点シェーダー・ピクセルシェーダー共通部分。
// DxLibVS.hlsli・DxLibPS.hlsli から読み込まれる。直接 include しなくてよい。
//
// 並びと名前は DxLib のソースパッケージの Windows/DxShader_Common_D3D11.h と同じ(変えないこと)。
// 各フィールドの意味は 06_D3D11Shader/spec/dxlib_d3d11_shader_spec.html の 8.1 章。

#ifndef DXLIB_COMMON_HLSLI
#define DXLIB_COMMON_HLSLI

#define DX_LIGHTTYPE_POINT       1
#define DX_LIGHTTYPE_SPOT        2
#define DX_LIGHTTYPE_DIRECTIONAL 3

#define DX_D3D11_COMMON_CONST_LIGHT_NUM 6	// ライトの最大数

// マテリアル
struct DX_D3D11_CONST_MATERIAL
{
	float4 Diffuse;				// ディフューズ色
	float4 Specular;			// スペキュラー色
	float4 Ambient_Emissive;	// エミッシブ色 + マテリアルのアンビエント色 × グローバルアンビエント色
	float Power;				// スペキュラーの強さ
	float TypeParam0;			// マテリアルの種類ごとのパラメーター 0
	float TypeParam1;			// マテリアルの種類ごとのパラメーター 1
	float TypeParam2;			// マテリアルの種類ごとのパラメーター 2
};

// フォグ
struct DX_D3D11_VS_CONST_FOG
{
	int Mode;			// フォグの種類(DX_FOGMODE_*)。描画の順番で変わるので使わない(仕様書 8.1)
	int3 Padding1;
	float LinearAdd;	// 線形フォグ: end / (end - start)
	float LinearDiv;	// 線形フォグ: -1 / (end - start)
	float Density;		// 指数フォグの密度
	float DensityStart;	// 0
	float E;			// 自然対数の底 2.71828183
	float3 Padding2;
	float4 Color;		// フォグの色
};

// ライト(位置・向きはビュー空間)
struct DX_D3D11_CONST_LIGHT
{
	int Type;			// DX_LIGHTTYPE_*。無効になったスロットでも前の値が残る(仕様書 8.1)
	int3 Padding1;
	float3 Position;	// 位置(ビュー空間)
	float RangePow2;	// 有効距離の 2 乗
	float3 Direction;	// 向き(ビュー空間)。CreateSpotLightHandle のライトは正規化されていないことがある
	float FallOff;		// スポットライトのフォールオフ
	float3 Diffuse;		// ディフューズ色
	float SpotParam0;	// cos(外側の角度 / 2)
	float3 Specular;	// スペキュラー色
	float SpotParam1;	// 1 / (cos(内側の角度 / 2) - cos(外側の角度 / 2))
	float4 Ambient;		// ライトのアンビエント色 × マテリアルのアンビエント色
	float Attenuation0;	// 距離による減衰の係数 0
	float Attenuation1;	// 距離による減衰の係数 1
	float Attenuation2;	// 距離による減衰の係数 2
	float Padding2;
};

// 共通の定数(頂点シェーダー・ピクセルシェーダーの両方の b0。同じ中身)
struct DX_D3D11_CONST_BUFFER_COMMON
{
	DX_D3D11_CONST_LIGHT Light[DX_D3D11_COMMON_CONST_LIGHT_NUM];	// 有効なライトが ディレクショナル → スポット → ポイント の順に詰めて入る
	DX_D3D11_CONST_MATERIAL Material;
	DX_D3D11_VS_CONST_FOG Fog;
	DX_D3D11_VS_CONST_FOG VerticalFog;	// 高さフォグ
};

cbuffer cbD3D11_CONST_BUFFER_COMMON : register(b0)
{
	DX_D3D11_CONST_BUFFER_COMMON g_Common;
};

#endif
