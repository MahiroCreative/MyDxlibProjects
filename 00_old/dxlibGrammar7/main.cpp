#pragma once
//オブジェクトヘッダ
#include "Bullet.h"
#include "Player.h"
#include "Enemy.h"
//共有ヘッダ(依存関係が強いものほど最後)
#include "MyDxlibCommon.h"
#include "math.h"
#include "DxLib.h"//何処でも使うので最後


/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//解説する内容は必要最低限です。抜けが出ます。また詳細な解説をしていません。
//随時リファレンスや外部サイトを参考にしてください。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・ゲームプログラミング
// 　- オブジェクト指向を用いたゲームプログラミング
//・C/C++言語
//   - クラス

/*コメント*/
//本来ならクラスを作る際に.hと.cppに分けるが、コードの簡便化のために全て.hに記述している。
//実際にゲーム作りの際には.hと.cppに分けてほしい。
//コンストラクタとか、ゲッターとかセッターとかの解説はここではしない。
//また、当たり判定のカラーコードなど、よく使う定数などは"Common.h"にまとめた。


//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(WindowSize::SIZE_X, WindowSize::SIZE_Y, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*ゲーム定数*/
	//Player.
	constexpr int PLAYER_FIRST_POSX = 20;
	constexpr int PLAYER_FIRST_POSY = 360;
	constexpr int PLAYER_R = 8;
	constexpr int PLAYER_SPEED = 3;
	constexpr int PLAYER_SHOT_MAX = 3;
	constexpr int PLAYER_SHOT_R = 2;
	constexpr int PLAYER_SHOT_SPEED = 8;
	//Enemy.
	constexpr int ENEMY_FIRST_POSX = 1100;
	constexpr int ENEMY_FIRST_POSY = 360;
	constexpr int ENEMY_R = 64;
	constexpr int ENEMY_SPEED = 2;
	constexpr int ENEMY_SHOT_R = 24;
	constexpr int ENEMY_SHOT_SPEED = -2;

	/*ゲーム変数*/
	int playerDrawHandle = LoadGraph("Chara.png");
	int enemyShotSpeed = 4;
	int enemyShotSize = 32;
	int delSpeed = 1;
	int delSize = 4;

	/*デバッグ用変数*/
	int debugNum = 0;
	int debugNum2 = 0;

	/*ポインタの作成*/
	//クラスは基本的にポインタで運用する。
	Player* pPlayer;
	Enemy* pEnemy;
	Bullet* pPBullet[PLAYER_SHOT_MAX];
	Bullet* pEBullet;

	/*インスタンスの作成*/
	pPlayer = new Player;
	pEnemy = new Enemy;
	pEBullet = new Bullet;
	//PlayerBullet.
	for (int i = 0; i < PLAYER_SHOT_MAX; i++)
	{
		pPBullet[i] = new Bullet;//インスタンス生成
	}

	/*初期化(PlayerとEnemyのみ)*/
	pPlayer->Init(LoadGraph("Chara.png"), PLAYER_FIRST_POSX, PLAYER_FIRST_POSY, PLAYER_R, PLAYER_SPEED, true, true);
	pEnemy->Init(ENEMY_FIRST_POSX, ENEMY_FIRST_POSY, ENEMY_R, ENEMY_SPEED, true);

	/*ゲームループ部*/
	//gameRoop.
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();
		//裏画面の初期化
		ClearDrawScreen();

		/*Update*/
		//player.
		pPlayer->Update();
		//enemy.
		pEnemy->Update();
		//eBullet.
		if (pEBullet->getVisible())
		{
			pEBullet->Update();
		}
		//pBulet.
		for (int i = 0; i < PLAYER_SHOT_MAX; i++)
		{
			if (pPBullet[i]->getVisible())
			{
				pPBullet[i]->Update();
			}

		}

		/*弾丸の発射*/
		//Player.
		if (pPlayer->getShotFlag())
		{
			for (int i = 0; i < PLAYER_SHOT_MAX; i++)
			{
				if (!pPBullet[i]->getVisible())
				{
					pPBullet[i]->Init(pPlayer->X, pPlayer->Y, PLAYER_SHOT_R, PLAYER_SHOT_SPEED, ColorCode::MAGENTA, true);
					break;
				}
			}
		}
		//Enemy.
		//画面から出たら弾が消ええるので、その際に再発射
		if (!pEBullet->getVisible())
		{
			pEBullet->Init(pEnemy->X, pEnemy->Y, enemyShotSize, -enemyShotSpeed, ColorCode::LIME, true);
			//弾の速度とサイズの更新
			enemyShotSpeed += delSpeed;
			enemyShotSize += delSize;
		}

		/*当たり判定*/
		///エネミーの弾とプレイヤーの当たり判定
		//距離の計算
		int delX = pPlayer->X - pEBullet->X;//xの相対距離
		int delY = pPlayer->Y - pEBullet->Y;//yの相対距離
		int magLen = delX * delX + delY * delY;//距離のマグニチュード(2乗)
		int delR = pPlayer->getR() + pEBullet->getR();//当たった時のR
		int magR = delR * delR;//Rのマグニチュード(2乗)
		//当たり判定の計算
		if (magLen < magR)
		{
			//当たってたらdebugNum変更
			debugNum = 1;
		}
		else
		{
			debugNum = 0;
		}
		///エネミーとPlayerの弾の当たり判定
		for (int i = 0; i < PLAYER_SHOT_MAX; i++)
		{
			//距離の計算
			delX = pEnemy->X - pPBullet[i]->X;//xの相対距離
			delY = pEnemy->Y - pPBullet[i]->Y;;//yの相対距離
			magLen = delX * delX + delY * delY;//距離のマグニチュード(2乗)
			delR = pEnemy->getR() + pPBullet[i]->getR();//当たった時のR
			magR = delR * delR;//Rのマグニチュード(2乗)
			//当たり判定の計算
			if (magLen < magR)
			{
				//当たってたらエネミーの色変える
				pEnemy->setColor(ColorCode::YELLOW);
				break;//一発でも当たってたら当たってた扱い。
			}
			else
			{
				pEnemy->setColor(ColorCode::AQUA);
			}w
		}

		/*Draw*/
		//player.
		pPlayer->Draw();
		//enemy.
		pEnemy->Draw();
		//eBullet.
		pEBullet->Draw();
		//pBullet.
		for (int i = 0; i < PLAYER_SHOT_MAX; i++)
		{
			pPBullet[i]->Draw();
		}

		/*DebguDraw*/
		//Debu文字
		DrawFormatString(0, 0, ColorCode::RED, "WSAD(上下左右),Enter(ショット)");
		DrawFormatString(0, 20, ColorCode::RED, "debugNum:%d", debugNum);
		//コリジョン表示
		pPlayer->DebugDraw();
		pEnemy->DebugDraw();

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
