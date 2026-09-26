// 検証用: ピクセルシェーダーに DxLib がセットした定数バッファの中身を、そのまま描画先に書き出す。
// x 番目の画素に、定数バッファの x 番目の 32 ビットを書く(下位 16 ビットを R、上位 16 ビットを G、B は「描いた印」の 0.5)。
// 入力は SV_POSITION だけを宣言する(2D でも 3D でも、DxLib の頂点シェーダーの出力の先頭は SV_POSITION)。
//
// コンパイル時の定義:
//   DUMP_SLOT  読む定数バッファのスロット(b0 など)
//   DUMP_REGS  定数バッファの大きさ(float4 何個分か)

cbuffer Dump : register(DUMP_SLOT)
{
	uint4 g_Regs[DUMP_REGS];
};

float4 main(float4 position : SV_POSITION) : SV_TARGET0
{
	const uint index = (uint)position.x;
	const uint reg = index / 4;
	const uint comp = index % 4;

	uint4 r = uint4(0, 0, 0, 0);
	if (reg < DUMP_REGS)
	{
		r = g_Regs[reg];
	}
	const uint v = comp == 0 ? r.x : comp == 1 ? r.y : comp == 2 ? r.z : r.w;
	return float4((float)(v & 0xFFFF), (float)(v >> 16), 0.5f, 1.0f);
}
