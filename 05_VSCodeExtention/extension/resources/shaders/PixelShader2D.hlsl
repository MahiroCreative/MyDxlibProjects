// 2D 用ピクセルシェーダー(Direct3D 11 / ps_4_0)
//
// 使い方(C++ 側):
//   int ps = LoadPixelShader("shaders/bin/__SHADER_NAME__.pso");
//   SetUsePixelShader(ps);
//   SetUseTextureToShader(0, 画像ハンドル);   // 下の g_Texture に入る
//   DrawPrimitive2DToShader(頂点(VERTEX2DSHADER), 頂点数, DX_PRIMTYPE_TRIANGLELIST);
//
// 2D では、頂点シェーダーは常に DxLib の既定のものが使われる(自作の頂点シェーダーを指定しても無視される)。
// DxLib はスペキュラー色(COLOR1)を含む下の PS_INPUT の並びで値を渡してくる。
// 使わない要素も消さず、並び順も変えないこと(ずれるとテクスチャ座標に別の値が入る)。
// 3D(DrawPolygon3DToShader)では並びが違うので、「ピクセルシェーダー(3D)」の雛形を使うこと。

Texture2D    g_Texture : register(t0);	// SetUseTextureToShader(0, ...) で指定した画像
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
	float4 Position   : SV_POSITION;	// 画面上の位置
	float4 Diffuse    : COLOR0;			// 頂点の色(dif)
	float4 Specular   : COLOR1;			// 頂点のスペキュラー色(spc)
	float2 TexCoords0 : TEXCOORD0;		// テクスチャ座標(u, v)
	float2 TexCoords1 : TEXCOORD1;		// サブテクスチャ座標(su, sv)
};

float4 main(PS_INPUT input) : SV_TARGET0
{
	float4 color = g_Texture.Sample(g_Sampler, input.TexCoords0);
	return color * input.Diffuse;
}
