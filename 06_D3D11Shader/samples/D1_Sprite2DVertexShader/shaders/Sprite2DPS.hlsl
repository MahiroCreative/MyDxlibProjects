// 2D の絵を描くピクセルシェーダー(正射影カメラ + 3D の板用)。Direct3D 11 / ps_4_0
// 画像の色に頂点の色(dif)を掛けるだけ。不透明度は頂点の色のアルファで渡す
// (自作シェーダーの描画では、SetDrawBlendMode の 2 番目の引数(不透明度)はシェーダーに届かない。仕様書 8.5)。

Texture2D    g_Texture : register(t0);	// SetUseTextureToShader( 0, 画像 )
SamplerState g_Sampler : register(s0);

// Sprite2DVS.hlsl の VS_OUTPUT と同じ並び
struct PS_INPUT
{
	float4 Position   : SV_POSITION;
	float4 Diffuse    : COLOR0;
	float2 TexCoords0 : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET0
{
	float4 color = g_Texture.Sample(g_Sampler, input.TexCoords0) * input.Diffuse;

	// DrawGraph( x, y, 画像, TRUE ) と同じく、透明な画素(アルファが 0)は描かない。
	// 画像の透過色(既定は黒)の画素は、LoadGraph で読み込んだときにアルファが 0 になっている。
	// これが無いと、SetDrawBlendMode が NOBLEND(既定)のとき、透過色の画素がそのまま黒く描かれる
	clip(color.a - 0.5f / 255.0f);

	return color;
}
