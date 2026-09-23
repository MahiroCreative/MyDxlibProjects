// 3D 用ピクセルシェーダー(Direct3D 11 / ps_4_0)
//
// 使い方(C++ 側):
//   int ps = LoadPixelShader("shaders/bin/__SHADER_NAME__.pso");
//   SetUsePixelShader(ps);
//   SetUseTextureToShader(0, 画像ハンドル);   // 下の g_Texture に入る
//   DrawPolygon3DToShader(頂点(VERTEX3DSHADER), ポリゴン数);
//
// 頂点シェーダーは、指定しなければ DxLib の既定のものが使われる。自作する場合は
// 「頂点シェーダー(3D)」の雛形を使うこと。どちらも下の PS_INPUT と同じ並びで値を渡す。
// 並び順を変えたり、途中の要素を消したりすると値がずれるので変えないこと。
// 2D(DrawPrimitive2DToShader)では並びが違うので、「ピクセルシェーダー(2D)」の雛形を使うこと。

Texture2D    g_Texture : register(t0);	// SetUseTextureToShader(0, ...) で指定した画像
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
	float4 Position   : SV_POSITION;	// 画面上の位置
	float4 Diffuse    : COLOR0;			// 頂点の色(dif)
	float2 TexCoords0 : TEXCOORD0;		// テクスチャ座標(u, v)
	float2 TexCoords1 : TEXCOORD1;		// サブテクスチャ座標(su, sv)
};

float4 main(PS_INPUT input) : SV_TARGET0
{
	float4 color = g_Texture.Sample(g_Sampler, input.TexCoords0);
	return color * input.Diffuse;
}
