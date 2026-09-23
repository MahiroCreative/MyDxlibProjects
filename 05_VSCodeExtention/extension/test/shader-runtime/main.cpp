// シェーダー雛形の実機検証。描画結果の画素を読んで results.txt に書き、PNG も保存して終了する。
#include "DxLib.h"
#include <cstdio>

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

static void Quad2D(float x0, float y0, float x1, float y1)
{
	VERTEX2DSHADER v[6] = {};
	const float px[6] = { x0, x1, x0, x1, x1, x0 };
	const float py[6] = { y0, y0, y1, y0, y1, y1 };
	const float pu[6] = { 0, 1, 0, 1, 1, 0 };
	const float pv[6] = { 0, 0, 1, 0, 1, 1 };
	for (int i = 0; i < 6; i++)
	{
		v[i].pos = VGet(px[i], py[i], 0.0f);
		v[i].rhw = 1.0f;
		v[i].dif = GetColorU8(255, 255, 255, 255);
		v[i].spc = GetColorU8(0, 0, 0, 0);
		v[i].u = pu[i];
		v[i].v = pv[i];
	}
	DrawPrimitive2DToShader(v, 6, DX_PRIMTYPE_TRIANGLELIST);
	RenderVertex();
}

static void Quad3D(float cy)
{
	VERTEX3DSHADER v[6] = {};
	const float px[6] = { -2, 2, -2, 2, 2, -2 };
	const float py[6] = { 1, 1, -1, 1, -1, -1 };
	const float pu[6] = { 0, 1, 0, 1, 1, 0 };
	const float pv[6] = { 0, 0, 1, 0, 1, 1 };
	for (int i = 0; i < 6; i++)
	{
		v[i].pos = VGet(px[i], py[i] + cy, 0.0f);
		v[i].norm = VGet(0, 0, -1);
		v[i].dif = GetColorU8(255, 255, 255, 255);
		v[i].spc = GetColorU8(0, 0, 0, 0);
		v[i].u = pu[i];
		v[i].v = pv[i];
	}
	DrawPolygon3DToShader(v, 2);
	RenderVertex();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	ChangeWindowMode(TRUE);
	SetGraphMode(640, 480, 32);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	fopen_s(&g_out, "results.txt", "w");
	fprintf(g_out, "Direct3D version=%d (3=D3D11)\n", GetUseDirect3DVersion());

	// 左半分が赤、右半分が緑の 64x64 の画像
	const int tex = MakeScreen(64, 64, FALSE);
	SetDrawScreen(tex);
	DrawBox(0, 0, 32, 64, GetColor(255, 0, 0), TRUE);
	DrawBox(32, 0, 64, 64, GetColor(0, 255, 0), TRUE);

	const int ps2d = LoadPixelShader("shaders/bin/Template2DPS.pso");
	const int ps2dInv = LoadPixelShader("shaders/bin/Invert2DPS.pso");
	const int ps3d = LoadPixelShader("shaders/bin/Template3DPS.pso");
	const int vs3d = LoadVertexShader("shaders/bin/TemplateVS.vso");
	fprintf(g_out, "handles ps2d=%d ps2dInv=%d ps3d=%d vs3d=%d\n", ps2d, ps2dInv, ps3d, vs3d);

	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();
	SetCameraNearFar(0.1f, 100.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(0, 0, -10), VGet(0, 0, 0));
	SetUseTextureToShader(0, tex);

	// 1. 2D: ピクセルシェーダー(2D 用雛形)
	SetUsePixelShader(ps2d);
	Quad2D(20, 20, 148, 84);
	// 2. 2D: 色反転版(自作シェーダーが動いている証明)
	SetUsePixelShader(ps2dInv);
	Quad2D(20, 100, 148, 164);
	// 3. 2D: 自作の頂点シェーダーを指定しても無視されること(雛形の説明の裏付け)
	SetUseVertexShader(vs3d);
	SetUsePixelShader(ps2d);
	Quad2D(20, 180, 148, 244);
	SetUseVertexShader(-1);
	// 4. 3D: ピクセルシェーダーだけ(頂点シェーダーは DxLib の既定)
	SetUsePixelShader(ps3d);
	Quad3D(-2.5f);
	// 5. 3D: 頂点シェーダーとピクセルシェーダーの両方(雛形どうし)
	SetUseVertexShader(vs3d);
	SetUsePixelShader(ps3d);
	Quad3D(0.0f);
	SetUseVertexShader(-1);
	SetUsePixelShader(-1);
	SetUseTextureToShader(0, -1);

	Check("2D_PS_left", 50, 52, 255, 0, 0);
	Check("2D_PS_right", 120, 52, 0, 255, 0);
	Check("2D_Invert_left", 50, 132, 0, 255, 255);
	Check("2D_Invert_right", 120, 132, 255, 0, 255);
	Check("2D_withVS_left", 50, 212, 255, 0, 0);
	Check("2D_withVS_right", 120, 212, 0, 255, 0);
	Check("3D_PSonly_left", 290, 344, 255, 0, 0);
	Check("3D_PSonly_right", 350, 344, 0, 255, 0);
	Check("3D_VSPS_left", 290, 240, 255, 0, 0);
	Check("3D_VSPS_right", 350, 240, 0, 255, 0);
	Check("background", 600, 20, 0, 0, 0);
	fprintf(g_out, "NG count=%d\n", g_ng);

	SaveDrawScreenToPNG(0, 0, 640, 480, "result.png");
	fclose(g_out);
	DxLib_End();
	return 0;
}
