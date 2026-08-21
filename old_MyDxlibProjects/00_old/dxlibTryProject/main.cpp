#include "DxLib.h"


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

	/*カメラ設定*/
	//奥行0.1～1000までをカメラの描画範囲とする
	SetCameraNearFar(0.1f, 1000.0f);
	//(0,10,-20)の視点から(0,10,0)のターゲットを見る角度にカメラを設置
	SetCameraPositionAndTarget_UpVecY(VGet(0, 10, -20), VGet(0.0f, 10.0f, 0.0f));


	/*モデル読込み*/
	int modelHandle;
	modelHandle = MV1LoadModel("data/model/player/hackadoll.pmx");

	/*ゲーム変数*/
	VECTOR pos;
	pos = VGet(0,0,0);

	/*ゲームループ部*/
	//gameRoop.
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		/*ゲーム処理部*/
		//モデル制御
		int key = GetJoypadInputState(DX_INPUT_KEY_PAD1);
		// 上を押していたら上に進む
		if (key & PAD_INPUT_UP)
		{
			pos.z += 1;
		}
		// 下を押していたら下に進む
		if (key & PAD_INPUT_DOWN)
		{
			pos.z -= 1;
		}
		// 右を押していたら右に進む
		if (key & PAD_INPUT_RIGHT)
		{
			pos.x += 1;
		}
		// 左を押していたら左に進む
		if (key & PAD_INPUT_LEFT)
		{
			pos.x -= 1;
		}
		//モデル位置更新
		MV1SetPosition(modelHandle, pos);

		/*描画*/
		//裏画面の初期化
		ClearDrawScreen();
		//モデル描画
		MV1DrawModel(modelHandle);
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
	DxLib::DxLib_End();//Dxlib終了処理
	return 0;//終了 
}
