#include "DxLib.h"
#include "player.h"

// Visual Studio 2026 で作ったプロジェクト(BOM なし UTF-8、文字セットは Unicode)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	SetDrawScreen(DX_SCREEN_BACK);
	while (ProcessMessage() == 0 && ClearDrawScreen() == 0)
	{
		DrawString(16, 16, L"VsGame", GetColor(255, 255, 255));
		ScreenFlip();
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
		{
			break;
		}
	}
	DxLib_End();
	return 0;
}
