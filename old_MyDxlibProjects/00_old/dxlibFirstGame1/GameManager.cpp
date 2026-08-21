#include <memory>
#include "DxLib.h"
#include "GameCommon.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "RankingScene.h"

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*定数*/
	//画面サイズ
	constexpr int ScreenSizeX = 1280;//幅
	constexpr int ScreenSizeY = 720;//高さ

	/*変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(ScreenSizeX, ScreenSizeY, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*Sceneの作成*/
	auto p_titleScene = std::make_unique<TitleScene>();
	auto p_gameScene = std::make_unique<GameScene>();
	auto p_rankingScene = std::make_unique<RankingScene>();

	/*ゲームループ部*/
	//Debug変数
	SceneKind firstScene = SceneKind::TITLESCENE;
	//Scene変数
	SceneKind nowScene = firstScene;
	SceneKind nextScene = firstScene;
	//gameRoop.
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		//裏画面の初期化
		ClearDrawScreen();

		/*ゲーム処理部*/
		if (nowScene == SceneKind::TITLESCENE)
		{
			//入出力処理
			MyKeyInput::Update();//Keybord.
			MyMouseInput::Update();//Mouse.
			//計算処理
			nextScene = p_titleScene->Update();
			//描画処理
			p_titleScene->Draw();
			//Debug処理
		}
		else if (nowScene == SceneKind::GAMESCENE)
		{
			//入出力処理
			MyKeyInput::Update();//Keybord.
			MyMouseInput::Update();//Mouse.
			//計算処理
			nextScene = p_gameScene->Update();
			//描画処理
			p_gameScene->Draw();
			//Debug処理
			p_gameScene->DebugDraw();
		}
		else if (nowScene == SceneKind::RANKING)
		{
			//入出力処理
			MyKeyInput::Update();//Keybord.
			MyMouseInput::Update();//Mouse.
			//計算処理
			nextScene = p_rankingScene->Update();
			//描画処理
			p_rankingScene->Draw();
			//Debug処理
			p_rankingScene->DebugDraw();
		}
		else if (nowScene == SceneKind::GAMEEND)
		{
			gameRoop = false;
		}

		/*Scene変更初期化処理*/
		if (nextScene != nowScene)
		{
			//シーン切り替え
			nowScene = nextScene;
			//それぞれのシーンの初期化処理
			if (nowScene == SceneKind::TITLESCENE)
			{
				p_titleScene->Init();
			}
			else if(nowScene == SceneKind::GAMESCENE)
			{
				p_gameScene->Init();
			}
			else if (nowScene == SceneKind::RANKING)
			{
				p_rankingScene->Init();
			}
		}


		//裏画面を表へ
		ScreenFlip();

		//リフレッシュ処理(-1ならエラー)
		if (ProcessMessage() < 0) { break; }

		//ループ終了処理
		if (CheckHitKey(KEY_INPUT_ESCAPE)) { break; }

		//fps固定(60fps:16.66ms)
		//ループ開始時刻から16.66ms経つまで停止
		while (GetNowHiPerformanceCount() - roopStartTime < 16667) {}
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//終了 
}