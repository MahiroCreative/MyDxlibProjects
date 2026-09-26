// 検証: 2D の絵を「正射影カメラ + 3D の板」(samples/D1_Sprite2DVertexShader の Camera2D.h とシェーダー)で描いたとき、
// DrawGraph で描いた絵と画素単位で同じになるか。結果は results_ortho2d.txt(OK/NG の判定)。
#include "DxLib.h"
#include "Camera2D.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>

static FILE* g_res = nullptr;
static int g_ng = 0;
static int g_screenA = -1;
static int g_screenB = -1;
static int g_vs = -1;
static int g_ps = -1;
static int g_cb = -1;
static int g_kao = -1;
static int g_glow = -1;
static int g_tex2 = -1;

static void Result(bool ok, const char* name, const std::string& detail)
{
	if (!ok)
	{
		g_ng++;
	}
	fprintf(g_res, "%s %s %s\n", ok ? "OK" : "NG", name, detail.c_str());
	fflush(g_res);
}

// 画面を背景色で消してから draw を呼び、中身を返す(ソフトウエアイメージ)
static int Capture(int screen, const std::function<void()>& draw)
{
	SetDrawScreen(screen);
	SetBackgroundColor(40, 40, 60);
	ClearDrawScreen();
	draw();
	const int si = MakeXRGB8ColorSoftImage(640, 480);
	GetDrawScreenSoftImage(0, 0, 640, 480, si);
	return si;
}

// 2 枚の違い: 最大の差と、差が tolerance を超えた画素の数
static void Compare(int a, int b, int& maxDiff, int& count, int tolerance)
{
	maxDiff = 0;
	count = 0;
	for (int y = 0; y < 480; y++)
	{
		for (int x = 0; x < 640; x++)
		{
			int r1, g1, b1, a1, r2, g2, b2, a2;
			GetPixelSoftImage(a, x, y, &r1, &g1, &b1, &a1);
			GetPixelSoftImage(b, x, y, &r2, &g2, &b2, &a2);
			const int d = max(abs(r1 - r2), max(abs(g1 - g2), abs(b1 - b2)));
			maxDiff = max(maxDiff, d);
			if (d > tolerance)
			{
				count++;
			}
		}
	}
}

// 正射影カメラ + 板で描く(Camera2D.h の関数とサンプルのシェーダー)
static void DrawSprite(float x, float y, float w, float h, int graph, COLOR_U8 color, float amplitude, int div)
{
	SetupCamera2D();
	float* p = (float*)GetBufferShaderConstantBuffer(g_cb);
	p[0] = 1.0f;			// 時間
	p[1] = amplitude;		// ゆれの大きさ
	p[2] = 128.0f;			// 波の長さ
	p[3] = 3.0f;			// 速さ
	UpdateShaderConstantBuffer(g_cb);
	SetShaderConstantBuffer(g_cb, DX_SHADERTYPE_VERTEX, 4);
	SetUseVertexShader(g_vs);
	SetUsePixelShader(g_ps);
	SetUseTextureToShader(0, graph);
	std::vector<VERTEX3DSHADER> v = MakeSpriteGrid(x, y, w, h, div, div, color);
	DrawPolygon3DToShader(v.data(), (int)v.size() / 3);
	SetUseTextureToShader(0, -1);
}

static void Check(const char* name, const std::function<void()>& reference, const std::function<void()>& test, int tolerance, bool expectSame = true)
{
	const int a = Capture(g_screenA, reference);
	const int b = Capture(g_screenB, test);
	// 調べやすいように、比べた 2 枚を保存しておく(項目の番号順)
	static int index = 0;
	index++;
	SaveSoftImageToPng((std::string("ortho2d_") + std::to_string(index) + "_ref.png").c_str(), a, 0);
	SaveSoftImageToPng((std::string("ortho2d_") + std::to_string(index) + "_test.png").c_str(), b, 0);
	int maxDiff, count;
	Compare(a, b, maxDiff, count, tolerance);
	const bool ok = expectSame ? count == 0 : count > 0;
	Result(ok, name, "max diff=" + std::to_string(maxDiff) + " pixels over " + std::to_string(tolerance) + "=" + std::to_string(count));
	DeleteSoftImage(a);
	DeleteSoftImage(b);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	ChangeWindowMode(TRUE);
	SetGraphMode(640, 480, 32);
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	SetOutApplicationLogValidFlag(FALSE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	fopen_s(&g_res, "results_ortho2d.txt", "w");

	g_screenA = MakeScreen(640, 480, FALSE);
	g_screenB = MakeScreen(640, 480, FALSE);
	g_vs = LoadVertexShader("shaders/Sprite2DVS.vso");
	g_ps = LoadPixelShader("shaders/Sprite2DPS.pso");
	g_cb = CreateShaderConstantBuffer(sizeof(float) * 4);
	g_kao = LoadGraph("Kao.bmp");
	g_glow = LoadGraph("Src2.tga");
	g_tex2 = LoadGraph("Tex2.bmp");
	Result(g_vs != -1 && g_ps != -1 && g_cb != -1 && g_kao != -1 && g_glow != -1 && g_tex2 != -1, "ortho2d/load", "");

	const COLOR_U8 white = GetColorU8(255, 255, 255, 255);

	// 1. 等倍・補間なし: 位置・向き(左右・上下の反転が無い)・にじみが無いこと。Kao.bmp は左向きの顔で左右非対称
	SetDrawMode(DX_DRAWMODE_NEAREST);
	Check("ortho2d/same as DrawGraph (1x, nearest)",
		[] { DrawGraph(100, 50, g_kao, TRUE); },
		[&] { DrawSprite(100, 50, 64, 64, g_kao, white, 0.0f, 8); }, 0);

	// 2. 4 倍・補間なし(ドット絵の拡大)
	Check("ortho2d/same as DrawExtendGraph (4x, nearest)",
		[] { DrawExtendGraph(100, 50, 356, 306, g_kao, TRUE); },
		[&] { DrawSprite(100, 50, 256, 256, g_kao, white, 0.0f, 16); }, 0);

	// 3. 4 倍・補間あり(透過色の黒を含まない画像)
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	Check("ortho2d/same as DrawExtendGraph (4x, bilinear, no transparent color)",
		[] { DrawExtendGraph(100, 50, 356, 306, g_tex2, TRUE); },
		[&] { DrawSprite(100, 50, 256, 256, g_tex2, white, 0.0f, 16); }, 2);

	// 3b. 知っておくこと: 透過色(黒)のある画像を補間ありで拡大すると、縁が DrawGraph と違う
	//     (自作のシェーダーでは、透過色の画素の黒が補間に混ざって縁が黒くなる。DrawGraph ではならない)
	Check("ortho2d/KNOWN: transparent-color edges differ from DrawExtendGraph when bilinear",
		[] { DrawExtendGraph(100, 50, 356, 306, g_kao, TRUE); },
		[&] { DrawSprite(100, 50, 256, 256, g_kao, white, 0.0f, 16); }, 2, false);
	SetDrawMode(DX_DRAWMODE_NEAREST);

	// 4. アルファ付きの画像を半透明で重ねる(SetDrawBlendMode の合成の方法は効く)
	Check("ortho2d/alpha blend with texture alpha (SetDrawBlendMode ALPHA)",
		[] { DrawGraph(100, 50, g_kao, TRUE); SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); DrawGraph(60, 20, g_glow, TRUE); SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); },
		[&] {
			DrawGraph(100, 50, g_kao, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
			DrawSprite(60, 20, 256, 256, g_glow, white, 0.0f, 4);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		},
		1);

	// 5. 不透明度: DrawGraph の SetDrawBlendMode( ALPHA, 128 ) と、自作シェーダーで頂点のアルファを 128 にしたもの
	Check("ortho2d/opacity by vertex alpha = SetDrawBlendMode(ALPHA, 128)",
		[] { DrawGraph(60, 20, g_glow, TRUE); SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); DrawExtendGraph(100, 50, 356, 306, g_kao, TRUE); SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); },
		[&] {
			DrawGraph(60, 20, g_glow, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
			DrawSprite(100, 50, 256, 256, g_kao, GetColorU8(255, 255, 255, 128), 0.0f, 16);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		},
		2);

	// 6. 正射影カメラで描いた後も、普通の 2D の描画(DrawGraph)は変わらない
	Check("ortho2d/DrawGraph after ortho drawing is unchanged",
		[] { DrawGraph(100, 50, g_kao, TRUE); DrawGraph(400, 300, g_kao, TRUE); },
		[&] { DrawSprite(100, 50, 64, 64, g_kao, white, 0.0f, 8); DrawGraph(400, 300, g_kao, TRUE); }, 0);

	// 7. ゆれ: 大きさを 0 以外にすると絵が変わる
	Check("ortho2d/wave moves vertices (amplitude 20 differs from 0)",
		[&] { DrawSprite(100, 50, 256, 256, g_kao, white, 0.0f, 16); },
		[&] { DrawSprite(100, 50, 256, 256, g_kao, white, 20.0f, 16); }, 0, false);

	// 8. ゆれ: 左端(u = 0)は留まっている(左端の 4 列だけを比べる)
	{
		const int a = Capture(g_screenA, [&] { DrawSprite(100, 50, 256, 256, g_kao, white, 0.0f, 16); });
		const int b = Capture(g_screenB, [&] { DrawSprite(100, 50, 256, 256, g_kao, white, 20.0f, 16); });
		int count = 0;
		for (int y = 50; y < 306; y++)
		{
			for (int x = 100; x < 104; x++)
			{
				int r1, g1, b1, a1, r2, g2, b2, a2;
				GetPixelSoftImage(a, x, y, &r1, &g1, &b1, &a1);
				GetPixelSoftImage(b, x, y, &r2, &g2, &b2, &a2);
				count += (r1 != r2 || g1 != g2 || b1 != b2);
			}
		}
		Result(count == 0, "ortho2d/wave keeps the left edge (u = 0) in place", "changed pixels in left 4 columns=" + std::to_string(count));
		DeleteSoftImage(a);
		DeleteSoftImage(b);
	}

	fprintf(g_res, "NG count=%d\n", g_ng);
	fclose(g_res);
	DxLib_End();
	return 0;
}
