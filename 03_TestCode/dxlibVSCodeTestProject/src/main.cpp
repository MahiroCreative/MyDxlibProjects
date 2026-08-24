#include "DxLib.h"
#include "Game.h"

// Windowsアプリのエントリーポイント。DxLibを使う場合は main ではなく WinMain を使う。
// 中身はGameクラスに任せ、ここでは初期化と実行だけを行う。
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Game& game = Game::GetInstance();

	if (!game.Initialize()) return -1;

	game.Run();

	return 0;
}
