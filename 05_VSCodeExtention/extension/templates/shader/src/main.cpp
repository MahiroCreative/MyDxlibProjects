#include "DxLib.h"

// __PROJECT_NAME__ のエントリーポイント。
// 自作のシェーダーで、左に 2D の四角形、右に 3D の板を描きます。
// シェーダーは shaders フォルダの 3 本です(何もしない雛形。画像の色をそのまま出す)。
//   Sample_2DPS.hlsl: 2D 用のピクセルシェーダー
//   Sample_3DVS.hlsl: 3D 用の頂点シェーダー
//   Sample_3DPS.hlsl: 3D 用のピクセルシェーダー
// ビルドすると shaders/bin にコンパイルされ(.pso と .vso)、ここで読み込みます。

// 見本の画像(4 色の格子)を作る。自分の画像を使うときは LoadGraph("image/xxx.png") に置き換える
int MakeSampleImage()
{
	const int image = MakeScreen(64, 64, FALSE);
	SetDrawScreen(image);
	DrawBox(0, 0, 32, 32, GetColor(255, 96, 96), TRUE);
	DrawBox(32, 0, 64, 32, GetColor(96, 255, 96), TRUE);
	DrawBox(0, 32, 32, 64, GetColor(96, 96, 255), TRUE);
	DrawBox(32, 32, 64, 64, GetColor(255, 255, 255), TRUE);
	SetDrawScreen(DX_SCREEN_BACK);
	return image;
}

// 2D の四角形を、今のピクセルシェーダーで描く(左上 x0, y0 から右下 x1, y1 まで。画像全体を貼る)
// 2D では頂点シェーダーは使えません(DxLib の既定のものが使われる)
void DrawQuad2D(float x0, float y0, float x1, float y1)
{
	// 三角形 2 枚で四角形にする。1 枚目: 左上・右上・左下、2 枚目: 右上・右下・左下
	const float x[6] = {x0, x1, x0, x1, x1, x0};
	const float y[6] = {y0, y0, y1, y0, y1, y1};
	const float u[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
	const float v[6] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};
	VERTEX2DSHADER vertex[6] = {};
	for (int i = 0; i < 6; i++)
	{
		vertex[i].pos = VGet(x[i], y[i], 0.0f);
		vertex[i].rhw = 1.0f;
		vertex[i].dif = GetColorU8(255, 255, 255, 255);
		vertex[i].spc = GetColorU8(0, 0, 0, 0);
		vertex[i].u = u[i];
		vertex[i].v = v[i];
	}
	DrawPolygon2DToShader(vertex, 2);
}

// 3D の板を、今の頂点シェーダーとピクセルシェーダーで描く(z = 0 の面に置く。画像全体を貼る)
// DxLib の既定のカメラでは、z = 0 の面の 1 は画面の 1 ドットに当たる。ただし y は上向き(画面の下が 0)
void DrawBoard3D(float x0, float y0, float x1, float y1)
{
	// 三角形 2 枚で四角形にする。1 枚目: 左上・右上・左下、2 枚目: 右上・右下・左下
	const float x[6] = {x0, x1, x0, x1, x1, x0};
	const float y[6] = {y1, y1, y0, y1, y0, y0};
	const float u[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
	const float v[6] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};
	VERTEX3DSHADER vertex[6] = {};
	for (int i = 0; i < 6; i++)
	{
		vertex[i].pos = VGet(x[i], y[i], 0.0f);
		vertex[i].norm = VGet(0.0f, 0.0f, -1.0f);
		vertex[i].dif = GetColorU8(255, 255, 255, 255);
		vertex[i].spc = GetColorU8(0, 0, 0, 0);
		vertex[i].u = u[i];
		vertex[i].v = v[i];
	}
	DrawPolygon3DToShader(vertex, 2);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウのタイトル
	SetMainWindowText("__PROJECT_NAME__");
	ChangeWindowMode(TRUE);
	SetGraphMode(1280, 720, 32);

	if (DxLib_Init() == -1)
	{
		return -1;
	}

	SetDrawScreen(DX_SCREEN_BACK);

	// シェーダーを読み込む(読み込めなければ -1)
	const int pixelShader2D = LoadPixelShader("shaders/bin/Sample_2DPS.pso");
	const int vertexShader3D = LoadVertexShader("shaders/bin/Sample_3DVS.vso");
	const int pixelShader3D = LoadPixelShader("shaders/bin/Sample_3DPS.pso");
	const int image = MakeSampleImage();

	while (ProcessMessage() == 0 && ClearDrawScreen() == 0)
	{
		if (pixelShader2D == -1 || vertexShader3D == -1 || pixelShader3D == -1)
		{
			DrawString(16, 16, "シェーダーを読み込めませんでした。ビルドの出力を確認してください。", GetColor(255, 96, 96));
		}
		else
		{
			// シェーダーの g_Texture(register(t0))に画像を渡す
			SetUseTextureToShader(0, image);

			// 2D: 左の四角形
			SetUsePixelShader(pixelShader2D);
			DrawQuad2D(160.0f, 200.0f, 480.0f, 520.0f);

			// 3D: 右の板(画面の x が 800 から 1120、y が 200 から 520 のあたりに出る)
			SetUseVertexShader(vertexShader3D);
			SetUsePixelShader(pixelShader3D);
			DrawBoard3D(800.0f, 200.0f, 1120.0f, 520.0f);

			// 使い終わったら元に戻す(この後の DrawString などに影響しないように)
			SetUseVertexShader(-1);
			SetUsePixelShader(-1);
			SetUseTextureToShader(0, -1);
		}

		DrawString(16, 680, "__PROJECT_NAME__", GetColor(255, 255, 255));

		ScreenFlip();

		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
		{
			break;
		}
	}

	DxLib_End();
	return 0;
}
