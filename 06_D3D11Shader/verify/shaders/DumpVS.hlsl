// 検証用: 頂点シェーダーに DxLib がセットした定数バッファの中身を、そのまま描画先に書き出す。
// 点を 1 個描くごとに、定数バッファの 32 ビット 1 個を 1 画素に書く(下位 16 ビットを R、上位 16 ビットを G)。
// 描画先は 32 ビット浮動小数点の画面。16 ビットの整数は float で誤差なく表せるので、C++ 側でビット列を復元できる。
//
// コンパイル時の定義:
//   DUMP_SLOT  読む定数バッファのスロット(b0 など)
//   DUMP_REGS  定数バッファの大きさ(float4 何個分か)
//   DUMP_WIDTH 描画先の幅(画素)

// DrawPrimitive3DToShader の頂点(VERTEX3DSHADER)。使わない要素も、全部を同じ順番で宣言する
struct VS_INPUT
{
	float3 Position : POSITION0;	// x に「何番目の 32 ビットか」を入れて渡す
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

cbuffer Dump : register(DUMP_SLOT)
{
	uint4 g_Regs[DUMP_REGS];
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	const uint index = (uint)input.Position.x;
	const uint reg = index / 4;
	const uint comp = index % 4;

	uint4 r = uint4(0, 0, 0, 0);
	if (reg < DUMP_REGS)
	{
		r = g_Regs[reg];
	}
	output.Value = comp == 0 ? r.x : comp == 1 ? r.y : comp == 2 ? r.z : r.w;

	// index 番目の画素の中心(描画先は高さ 1 画素)
	output.Position = float4(((float)index + 0.5f) / DUMP_WIDTH * 2.0f - 1.0f, 0.0f, 0.5f, 1.0f);
	return output;
}
