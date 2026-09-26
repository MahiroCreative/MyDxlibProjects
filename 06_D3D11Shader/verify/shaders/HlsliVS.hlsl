// 検証用: samples/_shared/DxLibVS.hlsli の宣言(フィールド名)で定数を読み、DumpVS.hlsl と同じ形で書き出す。
// C++ 側で、同じ場面の生の中身(DumpVS の結果)の決まった位置と比べて、宣言の並びが DxLib と一致しているかを確かめる。
// 点 i が、下の表の (i / 4) 番目の値の (i % 4) 成分を書く。

#include "DxLibVS.hlsli"

struct VS_INPUT
{
	float3 Position : POSITION0;
	float4 SubPosition : POSITION1;
	float3 Normal : NORMAL0;
	float3 Tangent : TANGENT0;
	float3 Binormal : BINORMAL0;
	float4 Diffuse : COLOR0;
	float4 Specular : COLOR1;
	float2 TexCoords0 : TEXCOORD0;
	float2 TexCoords1 : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	nointerpolation uint Value : TEXCOORD0;
};

uint4 Field(uint f)
{
	switch (f)
	{
	case 0: return uint4(asuint(g_Common.Light[3].Direction), asuint(g_Common.Light[3].FallOff));
	case 1: return uint4(asuint(g_Common.Light[5].Attenuation0), asuint(g_Common.Light[5].Attenuation1), asuint(g_Common.Light[5].Attenuation2), asuint(g_Common.Light[5].Padding2));
	case 2: return uint4(asint(g_Common.Light[5].Type), asint(g_Common.Light[5].Padding1));
	case 3: return uint4(asuint(g_Common.Material.Power), asuint(g_Common.Material.TypeParam0), asuint(g_Common.Material.TypeParam1), asuint(g_Common.Material.TypeParam2));
	case 4: return asuint(g_Common.Fog.Color);
	case 5: return uint4(asuint(g_Common.VerticalFog.LinearAdd), asuint(g_Common.VerticalFog.LinearDiv), asuint(g_Common.VerticalFog.Density), asuint(g_Common.VerticalFog.DensityStart));
	case 6: return asuint(g_Base.ProjectionMatrix[3]);
	case 7: return asuint(g_Base.ViewMatrix[2]);
	case 8: return asuint(g_Base.LocalWorldMatrix[2]);
	case 9: return uint4(asuint(g_Base.DiffuseSource), asuint(g_Base.SpecularSource), asuint(g_Base.MulSpecularColor), asuint(g_Base.Padding));
	case 10: return asuint(g_OtherMatrix.ShadowMapLightViewProjectionMatrix[2][3]);
	case 11: return asuint(g_OtherMatrix.TextureMatrix[0][1]);
	default: return asuint(g_LocalWorldMatrix.Matrix[161]);
	}
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	const uint index = (uint)input.Position.x;
	const uint4 v = Field(index / 4);
	const uint comp = index % 4;
	output.Value = comp == 0 ? v.x : comp == 1 ? v.y : comp == 2 ? v.z : v.w;
	output.Position = float4(((float)index + 0.5f) / DUMP_WIDTH * 2.0f - 1.0f, 0.0f, 0.5f, 1.0f);
	return output;
}
