// DxLib 3.24f(Direct3D 11)が自作の頂点シェーダーに渡す定数(b0〜b3)。
// 頂点シェーダーの先頭で #include "DxLibVS.hlsli" する。
//
// 並びと名前は DxLib のソースパッケージの Windows/DxShader_VS_D3D11.h と同じ(変えないこと)。
// 各フィールドの意味は 06_D3D11Shader/spec/dxlib_d3d11_shader_spec.md の 8 章。
//
// 行列は DxLib の MATRIX を転置して入っている。掛け方は dot(float4(座標, 1), 行列[k])(仕様書 7 章)。
// 自作の定数バッファは b4 以降に置く(b0〜b3 は DxLib が使っている)。

#ifndef DXLIB_VS_HLSLI
#define DXLIB_VS_HLSLI

#include "DxLibCommon.hlsli"

#define DX_D3D11_VS_CONST_TEXTURE_MATRIX_NUM 3	// テクスチャ座標の変換行列の数(使われるのは 0 番だけ)
#define DX_D3D11_VS_CONST_WORLD_MAT_NUM 54		// 1 つのトライアングルリストで使えるボーンの数

// 基本の定数(b1)
struct DX_D3D11_VS_CONST_BUFFER_BASE
{
	float4 AntiViewportMatrix[4];	// 画素座標 → 射影空間(使わなくてよい)
	float4 ProjectionMatrix[4];		// ビュー → 射影(2D の描画では 画素座標 → 射影空間)
	float4 ViewMatrix[3];			// ワールド → ビュー(3 行だけ)
	float4 LocalWorldMatrix[3];		// ローカル → ワールド(3 行だけ。MV1 のスキンメッシュでは使われない)
	float4 ToonOutLineSize;			// トゥーンの輪郭線の太さ
	float DiffuseSource;			// 1: 頂点のディフューズ色を使う 0: マテリアルの色を使う
	float SpecularSource;			// 1: 頂点のスペキュラー色を使う 0: マテリアルの色を使う
	float MulSpecularColor;			// スペキュラーを使うなら 1、使わないなら 0
	float Padding;
};

// その他の行列(b2)
struct DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX
{
	float4 ShadowMapLightViewProjectionMatrix[3][4];						// シャドウマップのライトのビュー × 射影(ワールド座標に掛ける)
	float4 TextureMatrix[DX_D3D11_VS_CONST_TEXTURE_MATRIX_NUM][2];		// テクスチャ座標の変換行列(0 番の 0 列目・1 列目)
};

// スキンメッシュのボーン行列(b3)
struct DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX
{
	float4 Matrix[DX_D3D11_VS_CONST_WORLD_MAT_NUM * 3];	// ボーン j の行列が Matrix[j*3+0〜2]。頂点の BLENDINDICES はボーン番号 × 3
};

cbuffer cbD3D11_CONST_BUFFER_VS_BASE : register(b1)
{
	DX_D3D11_VS_CONST_BUFFER_BASE g_Base;
};

cbuffer cbD3D11_CONST_BUFFER_VS_OTHERMATRIX : register(b2)
{
	DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX g_OtherMatrix;
};

cbuffer cbD3D11_CONST_BUFFER_VS_LOCALWORLDMATRIX : register(b3)
{
	DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX g_LocalWorldMatrix;
};

#endif
