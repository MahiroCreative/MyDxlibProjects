// 検証用: samples/_shared/DxLibPS.hlsli の宣言(フィールド名)で定数を読み、DumpPS.hlsl と同じ形で書き出す。
// x 番目の画素が、下の表の (x / 4) 番目の値の (x % 4) 成分を書く。

#include "DxLibPS.hlsli"

uint4 Field(uint f)
{
	switch (f)
	{
	case 0: return asuint(g_Base.FactorColor);
	case 1: return uint4(asuint(g_Base.MulAlphaColor), asuint(g_Base.AlphaTestRef), asuint(g_Base.Padding1));
	case 2: return uint4(asint(g_Base.AlphaTestCmpMode), asint(g_Base.NoLightAngleAttenuation), asint(g_Base.UseHalfLambert), asint(g_Base.Padding2));
	case 3: return asuint(g_Base.DrawAddColor);
	case 4: return uint4(asuint(g_ShadowMap.Data[1].Enable_Light2), asuint(g_ShadowMap.Data[1].Padding));
	case 5: return uint4(asuint(g_ShadowMap.Data[2].AdjustDepth), asuint(g_ShadowMap.Data[2].GradationParam), asuint(g_ShadowMap.Data[2].Enable_Light0), asuint(g_ShadowMap.Data[2].Enable_Light1));
	default: return uint4(asuint(g_Common.Light[4].Specular), asuint(g_Common.Light[4].SpotParam1));
	}
}

float4 main(float4 position : SV_POSITION) : SV_TARGET0
{
	const uint index = (uint)position.x;
	const uint4 v = Field(index / 4);
	const uint comp = index % 4;
	const uint x = comp == 0 ? v.x : comp == 1 ? v.y : comp == 2 ? v.z : v.w;
	return float4((float)(x & 0xFFFF), (float)(x >> 16), 0.5f, 1.0f);
}
