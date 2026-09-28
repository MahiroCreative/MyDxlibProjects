// 段階 4: シェーダー入りテンプレート(templates/shader)の main.cpp を、そのまま動かして画素で判定する。
// main.cpp の ScreenFlip だけをこのファイルの TemplateCheckFlip に差し替え、最初の 1 フレームを判定して終わる。
// template_main.cpp は run.js がテンプレートの main.cpp を写したもの(__PROJECT_NAME__ を置換済み)。
#include "DxLib.h"
#include <cstdio>
#include <cstdlib>

static int TemplateCheckFlip();
#define ScreenFlip TemplateCheckFlip
#include "template_main.cpp"
#undef ScreenFlip

static FILE* g_out = nullptr;
static int g_ng = 0;

static void Check(const char* name, int x, int y, int er, int eg, int eb)
{
	int r, g, b;
	GetColor2(GetPixel(x, y), &r, &g, &b);
	const bool ok = (r - er) * (r - er) < 40 * 40 && (g - eg) * (g - eg) < 40 * 40 && (b - eb) * (b - eb) < 40 * 40;
	if (!ok)
	{
		g_ng++;
	}
	fprintf(g_out, "%s %s (%d,%d) got=(%d,%d,%d) expected=(%d,%d,%d)\n", ok ? "OK" : "NG", name, x, y, r, g, b, er, eg, eb);
}

static int TemplateCheckFlip()
{
	fopen_s(&g_out, "template_results.txt", "w");
	fprintf(g_out, "Direct3D version=%d (3=D3D11)\n", GetUseDirect3DVersion());
	// 見本の画像は 4 色の格子(左上 赤、右上 緑、左下 青、右下 白)。上下・左右が正しく貼られていることも見る
	// 2D の四角形 (160,200)-(480,520)
	Check("2D_topLeft_red", 240, 280, 255, 96, 96);
	Check("2D_topRight_green", 400, 280, 96, 255, 96);
	Check("2D_bottomLeft_blue", 240, 440, 96, 96, 255);
	Check("2D_bottomRight_white", 400, 440, 255, 255, 255);
	// 3D の板。既定のカメラで画面の (800,200)-(1120,520) に出る
	Check("3D_topLeft_red", 880, 280, 255, 96, 96);
	Check("3D_topRight_green", 1040, 280, 96, 255, 96);
	Check("3D_bottomLeft_blue", 880, 440, 96, 96, 255);
	Check("3D_bottomRight_white", 1040, 440, 255, 255, 255);
	// 板の外は背景(黒)。3D の板が大きさ・位置どおりであることの確認
	Check("3D_outside_left", 780, 360, 0, 0, 0);
	Check("3D_outside_right", 1140, 360, 0, 0, 0);
	Check("3D_outside_top", 960, 180, 0, 0, 0);
	Check("3D_outside_bottom", 960, 540, 0, 0, 0);
	Check("background", 640, 100, 0, 0, 0);
	fprintf(g_out, "NG count=%d\n", g_ng);
	SaveDrawScreenToPNG(0, 0, 1280, 720, "template_result.png");
	fclose(g_out);
	DxLib_End();
	exit(0);
}
