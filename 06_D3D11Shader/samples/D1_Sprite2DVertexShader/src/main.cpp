// 2D の絵を頂点シェーダーで動かす(旗のようになびかせる)サンプル。
//
// DxLib の 2D の描画(DrawGraph や DrawPrimitive2DToShader)では、自作の頂点シェーダーは使えない。
// そこで、カメラを「正射影・画面の画素と 3D の座標が 1 対 1」にして(Camera2D.h の SetupCamera2D)、
// 絵を 3D の板として DrawPolygon3DToShader で描く。こうすると 2D と同じ見た目のまま、頂点シェーダーで頂点を動かせる。
//
// 左: ふつうの DrawExtendGraph。右: 同じ絵を、頂点シェーダーでなびかせたもの。
// 上下キー: ゆれの大きさ。左右キー: 波の長さ。
#include "DxLib.h"
#include "Camera2D.h"

// 頂点シェーダーの定数バッファ(b4)の中身。Sprite2DVS.hlsl の cbuffer WaveParam と同じ並び(大きさは 16 バイトの倍数)
struct WAVE_PARAM
{
	float Time;			// 経過時間(秒)
	float Amplitude;	// ゆれの大きさ(画素)
	float WaveLength;	// 波の長さ(画素)
	float Speed;		// 波が進む速さ
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	ChangeWindowMode(TRUE);
	SetGraphMode(640, 480, 32);
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	const int image = LoadGraph("Kao.bmp");	// 64×64 の絵(黒は透過色)
	const int vs = LoadVertexShader("shaders/bin/Sprite2DVS.vso");
	const int ps = LoadPixelShader("shaders/bin/Sprite2DPS.pso");
	const int cb = CreateShaderConstantBuffer(sizeof(WAVE_PARAM));
	WAVE_PARAM* param = (WAVE_PARAM*)GetBufferShaderConstantBuffer(cb);
	param->Amplitude = 16.0f;
	param->WaveLength = 160.0f;
	param->Speed = 4.0f;

	// なびかせる板。縦横 24 マスに分けて頂点を増やす(4 隅だけではなめらかに曲がらない)
	const std::vector<VERTEX3DSHADER> flag = MakeSpriteGrid(344.0f, 112.0f, 256.0f, 256.0f, 24, 24, GetColorU8(255, 255, 255, 255));

	const int start = GetNowCount();
	SetDrawScreen(DX_SCREEN_BACK);
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		ClearDrawScreen();
		SetDrawMode(DX_DRAWMODE_NEAREST);	// ドット絵なので補間しない

		// キーでゆれを変える
		if (CheckHitKey(KEY_INPUT_UP)) param->Amplitude += 0.5f;
		if (CheckHitKey(KEY_INPUT_DOWN) && param->Amplitude > 0.0f) param->Amplitude -= 0.5f;
		if (CheckHitKey(KEY_INPUT_RIGHT)) param->WaveLength += 2.0f;
		if (CheckHitKey(KEY_INPUT_LEFT) && param->WaveLength > 20.0f) param->WaveLength -= 2.0f;

		// 左: ふつうの 2D の描画
		DrawExtendGraph(40, 112, 296, 368, image, TRUE);

		// 右: 正射影カメラ + 3D の板 + 自作の頂点シェーダー
		SetupCamera2D();	// SetDrawScreen の後に呼ぶ(SetDrawScreen はカメラの設定を元に戻すため)
		param->Time = (GetNowCount() - start) / 1000.0f;
		UpdateShaderConstantBuffer(cb);
		SetShaderConstantBuffer(cb, DX_SHADERTYPE_VERTEX, 4);
		SetUseVertexShader(vs);
		SetUsePixelShader(ps);
		SetUseTextureToShader(0, image);
		DrawPolygon3DToShader(flag.data(), (int)flag.size() / 3);
		SetUseTextureToShader(0, -1);

		// 3D の板の後でも、ふつうの 2D の描画はそのまま使える
		DrawString(40, 80, "DrawExtendGraph(ふつうの 2D)", GetColor(255, 255, 255));
		DrawString(344, 80, "頂点シェーダーでなびかせる", GetColor(255, 255, 255));
		DrawFormatString(40, 400, GetColor(200, 200, 200), "上下キー: ゆれの大きさ %.1f   左右キー: 波の長さ %.0f", param->Amplitude, param->WaveLength);

		ScreenFlip();
	}

	DeleteShaderConstantBuffer(cb);
	DeleteShader(vs);
	DeleteShader(ps);
	DeleteGraph(image);
	DxLib_End();
	return 0;
}
