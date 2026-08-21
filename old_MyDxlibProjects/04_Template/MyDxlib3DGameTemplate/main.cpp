#include "Dxlib.h"
#include "AppMain.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//メインループのインスタンス作成
	auto& app = Application::GetInstance();

	//初期化処理
	if (!app.Init()) { return -1; }//失敗した場合は終了
	
	//メインループ
	app.Run();

	//終了処理
	app.End();

	return 0;
}