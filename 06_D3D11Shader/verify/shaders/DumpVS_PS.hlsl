// 検証用: DumpVS.hlsl が渡した 32 ビットを、下位 16 ビット(R)と上位 16 ビット(G)に分けて書く。
// B は「描いた印」の 0.5、A は 1 にする(アルファブレンドや加算ブレンドが有効でも、黒で消した描画先には書いた値がそのまま残る)。

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	nointerpolation uint Value : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET0
{
	return float4((float)(input.Value & 0xFFFF), (float)(input.Value >> 16), 0.5f, 1.0f);
}
