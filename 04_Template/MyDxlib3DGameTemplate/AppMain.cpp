#include "AppMain.h"
#include "Dxlib.h"

//匿名名前空間は、同一ファイル内でのみ有効。
namespace
{
	/*静的設定*/
	// ウィンドウタイトル
	constexpr const char* WINDOW_TITLE = "MyDxlib3DGameTemplate";
	// ウィンドウサイズ
	constexpr int WINDOW_WIDTH = 1280;
	constexpr int WINDOW_HEIGHT = 720;
	//ウィンドウ中央座標
	constexpr int WINDOW_CENTER_X = WINDOW_WIDTH / 2;
	constexpr int WINDOW_CENTER_Y = WINDOW_HEIGHT / 2;

	/*動的設定*/
	//Windowモード
	bool isWindowMode = true;
}

//コンストラクタ
Application::Application() {}

//インスタンスを取得するための静的な関数
Application& Application::GetInstance()
{
	//静的なインスタンスを作成し、常に同じインスタンスを返す
	static Application app;
	return app;
}

//初期化処理
bool Application::Init()
{
	/*Dxlib初期化前処理*/
	ChangeWindowMode(isWindowMode);//ウィンドウモードの設定
	SetMainWindowText(WINDOW_TITLE);//ウィンドウタイトルの設定
	SetChangeScreenModeGraphicsSystemResetFlag(false);//画面モード変更時にグラフィックスをリセットしない
	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);//画面サイズと解像度

	//初期化
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化

	//ダブルバッファリング
	SetDrawScreen(DX_SCREEN_BACK);

	/*3D用処理*/
	SetUseZBuffer3D(true);//Zバッファの使用
	SetWriteZBuffer3D(true);//Zバッファへの書き込み許可

	//問題なく通過した場合はtrueを返す
	return true;
}

//ゲームの実行
void Application::Run()
{
	/*一時変数*/
	LONGLONG roopStartTime = 0;
	LONGLONG frameTime = 0;
	bool isGameRoop = true;

	/*ゲームループ*/
	while (isGameRoop)
	{
		/*roop開始部*/
		roopStartTime = GetNowHiPerformanceCount();//ループ開始時刻の確保
		ClearDrawScreen();//裏画面の初期化

		/*ゲーム部*/


		/*roop更新部*/		
		ScreenFlip();//裏画面を表へ
		if (ProcessMessage() < 0) { break; }//リフレッシュ処理(-1ならエラー)
		if (CheckHitKey(KEY_INPUT_ESCAPE)) { break; }//ループ終了処理
		frameTime = GetNowHiPerformanceCount() - roopStartTime;//現在の1frameにかかる時間を計測
		//fps固定(60fps:16.66ms)
		//ループ開始時刻から16.66ms経つまで停止
		while (GetNowHiPerformanceCount() - roopStartTime < 16667) {}

	}


}

//終了処理
void Application::End()
{
	/*終了処理*/
	DxLib_End();//Dxlib終了処理
}
