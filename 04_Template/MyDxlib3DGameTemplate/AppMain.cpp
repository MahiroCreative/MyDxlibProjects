//STL.
#include <memory>
//Dxlib.
#include "MyDxlib/MyDxlib.h"
//Origin.
#include "AppMain.h"
#include "GameSetting.h"
#include "TitleScene.h"

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
	ChangeWindowMode(GameSetting::isWindowMode);//ウィンドウモードの設定
	SetMainWindowText(GameSetting::WINDOW_TITLE);//ウィンドウタイトルの設定
	SetChangeScreenModeGraphicsSystemResetFlag(false);//画面モード変更時にグラフィックスをリセットしない
	SetGraphMode(GameSetting::WINDOW_WIDTH, GameSetting::WINDOW_HEIGHT, 32);//画面サイズと解像度

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

	/*ゲームシーンの生成*/
	std::unique_ptr<_baseGameScene> gameScene;
	gameScene = std::make_unique<TitleScene>();
	gameScene->Init();//初期化

	/*ゲームループ*/
	while (isGameRoop)
	{
		/*roop開始部*/
		roopStartTime = GetNowHiPerformanceCount();//ループ開始時刻の確保
		ClearDrawScreen();//裏画面の初期化

		/*ゲーム部*/
		gameScene->Update();//更新
		gameScene->Draw();//描画


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
