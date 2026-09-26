// DxLib 3.24f(Direct3D 11)が自作のピクセルシェーダーに渡す定数(b0〜b2)。
// ピクセルシェーダーの先頭で #include "DxLibPS.hlsli" する。
//
// 並びと名前は DxLib のソースパッケージの Windows/DxShader_PS_D3D11.h と同じ(変えないこと)。
// 各フィールドの意味は 06_D3D11Shader/spec/dxlib_d3d11_shader_spec.md の 8 章。
//
// 自作の定数バッファは b4 以降に置く(b0〜b2 と、フィルター用の b3 は DxLib が使っている)。

#ifndef DXLIB_PS_HLSLI
#define DXLIB_PS_HLSLI

#include "DxLibCommon.hlsli"

// 基本の定数(b1)
struct DX_D3D11_PS_CONST_BUFFER_BASE
{
	float4 FactorColor;				// MV1 の描画: (1, 1, 1, 不透明度)。DrawPolygon3DToShader などでは更新されない(仕様書 8.5)
	float MulAlphaColor;			// 色にアルファを掛けるか(MV1 の描画でだけ設定される)
	float AlphaTestRef;				// アルファテストの比較値(0〜1)
	float2 Padding1;
	int AlphaTestCmpMode;			// アルファテストの比較方法(DX_CMP_*)
	int NoLightAngleAttenuation;	// 1: ライトの角度による減衰をしない
	int UseHalfLambert;				// 1: ハーフランバートを使う
	int Padding2;
	float4 IgnoreTextureColor;		// テクスチャの色を無視する描画用
	float4 DrawAddColor;			// SetDrawAddColor の色(0〜1)
};

// シャドウマップ 1 枚分
struct DX_D3D11_PS_CONST_SHADOWMAP
{
	float AdjustDepth;		// 深度の比較の補正値
	float GradationParam;	// 影の境目のぼかしの範囲
	float Enable_Light0;	// 1: ライト 0 に影を「適用しない」、0: 適用する(名前と意味が逆)
	float Enable_Light1;	// 同上、ライト 1
	float Enable_Light2;	// 同上、ライト 2
	float3 Padding;
};

// シャドウマップの定数(b2)
struct DX_D3D11_PS_CONST_BUFFER_SHADOWMAP
{
	DX_D3D11_PS_CONST_SHADOWMAP Data[3];
};

cbuffer cbD3D11_CONST_BUFFER_PS_BASE : register(b1)
{
	DX_D3D11_PS_CONST_BUFFER_BASE g_Base;
};

cbuffer cbD3D11_CONST_BUFFER_PS_SHADOWMAP : register(b2)
{
	DX_D3D11_PS_CONST_BUFFER_SHADOWMAP g_ShadowMap;
};

#endif
