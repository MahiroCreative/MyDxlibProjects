//STL.
#include <iostream>
#include <memory>
//Dxlib.
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"
#include "TitleScene.h"
#include "ShootingScene.h"
#include "PlatformScene.h"
#include "ClearScene.h"
#include "OverScene.h"

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*変数*/
	LONGLONG roopStartTime = 0;
	LONGLONG frameTime = 0;
	int nowScene = 0;
	int nextScene = 0;
	bool gameRoop = true;

	/*Sceneオブジェクト生成*/
	std::unique_ptr<_baseGameScene> scene;
	scene = std::make_unique<PlatformScene>();//初期シーン
	scene->Init();//初期化
	 
	/*Dxlib初期化*/
	SetMainWindowText(GameSetting::WINDOW_TITLE);// ウィンドウタイトルを設定
	SetGraphMode(GameSetting::WINDOW_WIDTH, GameSetting::WINDOW_HEIGHT, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*GameRoop*/
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		//裏画面の初期化
		ClearDrawScreen();

		/*ゲーム部*/
		nextScene = scene->Update();//更新
		scene->Draw();//描画

		/*シーンの変更*/
		//ポリモーフィズム(多能性)を分かりやすくするために、
		// わざとAppMainのような物を作っていません。
		if (nowScene != nextScene)
		{
			//シーンの変更
			if (nextScene == static_cast<int>(GameSetting::SceneState::Title))
			{
				scene = std::make_unique<TitleScene>();//タイトルシーン
				scene->Init();//初期化
			}
			else if (nextScene == static_cast<int>(GameSetting::SceneState::ShootingGame))
			{
				scene = std::make_unique<ShootingScene>();//シューティングゲームシーン
				scene->Init();//初期化
			}
			else if (nextScene == static_cast<int>(GameSetting::SceneState::PlatformGame))
			{
				scene = std::make_unique<PlatformScene>();//プラットフォームゲームシーン
				scene->Init();//初期化
			}
			else if (nextScene == static_cast<int>(GameSetting::SceneState::ClearGame))
			{
				scene = std::make_unique<ClearScene>();
				scene->Init();//初期化
			}
			else if (nextScene == static_cast<int>(GameSetting::SceneState::OverGame))
			{
				scene = std::make_unique<OverScene>();//プラットフォームゲームシーン
				scene->Init();//初期化
			}
			else if (nextScene == static_cast<int>(GameSetting::SceneState::EndGame))
			{
				gameRoop = false;
			}

			//noSceneの更新
			nowScene = nextScene;
		}

		//裏画面を表へ
		ScreenFlip();

		//リフレッシュ処理(-1ならエラー)
		if (ProcessMessage() < 0) { break; }

		//ループ終了処理
		if (CheckHitKey(KEY_INPUT_ESCAPE)) { break; }

		//現在の1frameにかかる時間を計測
		frameTime = GetNowHiPerformanceCount() - roopStartTime;

		//fps固定(60fps:16.66ms)
		//ループ開始時刻から16.66ms経つまで停止
		while (GetNowHiPerformanceCount() - roopStartTime < 16667) {}
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//プログラム終了 
}