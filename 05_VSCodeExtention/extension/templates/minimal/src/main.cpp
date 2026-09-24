#include "DxLib.h"

// __PROJECT_NAME__ のエントリーポイント。
// DxLib を使う Windows アプリは main ではなく WinMain から始まります。
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウのタイトル
	SetMainWindowText("__PROJECT_NAME__");
	ChangeWindowMode(TRUE);      // TRUE でウィンドウモード、FALSE でフルスクリーン
	SetGraphMode(1280, 720, 32); // 画面サイズと色深度(DxLib_Init より前に呼ぶ)

	if (DxLib_Init() == -1) // DxLib の初期化。失敗したら終了
	{
		return -1;
	}

	SetDrawScreen(DX_SCREEN_BACK); // 裏画面に描いて ScreenFlip で表に出す(ちらつき防止)

	// メインループ。ウィンドウが閉じられるか ESC が押されたら抜ける
	while (ProcessMessage() == 0 && ClearDrawScreen() == 0)
	{
		DrawString(16, 16, "__PROJECT_NAME__", GetColor(255, 255, 255));

		ScreenFlip();

		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
		{
			break;
		}
	}

	DxLib_End(); // DxLib の終了処理
	return 0;
}
