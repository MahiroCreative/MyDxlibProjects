#include "DxLib.h"

/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//解説する内容は必要最低限です。抜けが出ます。また詳細な解説をしていません。
//随時リファレンスや外部サイトを参考にしてください。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・DxLib
//   - 時間の測定
// 　- fpsの固定
//・ゲームプログラミング
//   - fpsの固定
//・C/C++言語
//   - 構造体

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(1280, 720, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*構造体の作成*/
	//ゲームオブジェクト用の構造体
	//構造体は変数を１つにまとめれる。Playerやエネミーなど関連する変数を１つにまとめておこう。
	//Player.
	struct Player
	{
		int drawHandle;//画像ハンドル
		int posX;//X座標
		int posY;//Y座標
		int R;//半径
		int speed;//速度
		double scale;//スケール
		double rotate;//回転度
	};
	//Enemy.
	struct Enemy
	{
		int posX;//X座標
		int posY;//Y座標
		int R;//半径
		int speed;//速度
		unsigned int color;//色.iroGetColorで得られるのはunsigned int なので。
		unsigned int hitColor;//弾が当たった時の色
	};
	//Bullet.
	struct Bullet
	{
		int posX;//X座標
		int posY;//Y座標
		int R;//半径
		int speed;//速度
		unsigned int color;//色.
	};

	/*変数*/
	//Player.
	Player Player;
	Player.drawHandle = LoadGraph("Chara.png");
	Player.posX = 100;//x座標
	Player.posY = 300;//y座標
	Player.R = 2;//半径
	Player.speed = 2;//スピード
	Player.scale = 1;//スケール
	Player.rotate = 0;//回転度
	//Enemy.
	Enemy Enemy;
	Enemy.posX = 1000;
	Enemy.posY = 360;
	Enemy.R = 80;
	Enemy.speed = 1;
	Enemy.color = GetColor(255, 0, 255);
	Enemy.hitColor = GetColor(0, 0, 255);
	//PlayerBullet.
	Bullet pBullet;
	pBullet.posX = Player.posX;
	pBullet.posY = Player.posY;
	pBullet.R = 4;
	pBullet.speed = 8;
	pBullet.color = GetColor(255, 255, 255);
	//EnemyBullet.
	Bullet eBullet;
	eBullet.posX = Enemy.posX;
	eBullet.posY = Enemy.posY;
	eBullet.R = 32;
	eBullet.speed = 4;
	eBullet.color = GetColor(0, 255, 0);
	//フラグ用変数
	bool isPlayerBullet = false;
	bool isEnemyBullet = false;
	bool isEnemyHit = false;
	bool isPlayerHit = false;

	/*ゲームループ部*/
	//gameRoop.
	//ループの最初と最後に注目して欲しい。
	//それぞれGetNowHiPerformanceCount()で現在の時刻を取得している。
	//何をしているかというと、ループの開始時の時刻からループ終了時の時刻を引いている。
	//この値が16.66msになるまで次のループに行かないようにしている。
	//こうすることで、1秒間にループを何回回るかを制限できる。今回は60fpsにしている。
	//このように制限することでゲームの動作を一定の間隔に保つことができ、いわゆるラグというものの発生を減らせる。
	//(※前回までのコードだと弾の発射位置がたまにズレていた筈です。)
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		//裏画面の初期化
		ClearDrawScreen();

		/*Player処理*/
		//移動処理
		if (CheckHitKey(KEY_INPUT_W))//Wで上移動
		{
			Player.posY -= Player.speed;
		}
		else if (CheckHitKey(KEY_INPUT_S))//Sで下移動
		{
			Player.posY += Player.speed;
		}
		else if (CheckHitKey(KEY_INPUT_D))//Dで右移動
		{
			Player.posX += Player.speed;
		}
		else if (CheckHitKey(KEY_INPUT_A))//Aで左移動
		{
			Player.posX -= Player.speed;
		}
		//弾の発射
		if (CheckHitKey(KEY_INPUT_RETURN) && !isPlayerBullet)//弾は一発しか発射できない
		{
			isPlayerBullet = true;
		}

		/*Enemy処理*/
		//移動処理
		Enemy.posY += Enemy.speed;
		if (Enemy.posY > 640 || Enemy.posY < 80)//上下運動をさせている。
		{
			Enemy.speed = -Enemy.speed;
		}
		//弾の発射
		if (!isEnemyBullet)//弾は一発しか発射できない
		{
			isEnemyBullet = true;
			eBullet.speed += 1;//エネミーの弾の速度を撃つたびに早くする
		}

		/*PlayerBullet処理*/
		if (isPlayerBullet && pBullet.posX < 1280)//弾が発射されており、画面内なら移動
		{
			pBullet.posX += pBullet.speed;
		}
		else//画面外なら初期化
		{
			pBullet.posX = Player.posX;
			pBullet.posY = Player.posY;
			isPlayerBullet = false;
		}

		/*EnemyBullet処理*/
		if (isEnemyBullet)
		{
			eBullet.posX -= eBullet.speed;//移動処理
			//位置の初期化
			if (eBullet.posX < 0)
			{
				eBullet.posX = Enemy.posX;
				eBullet.posY = Enemy.posY;
				isEnemyBullet = false;
			}
		}

		/*当たり判定*/
		//Playerの当たり判定.
		int delX = Player.posX - eBullet.posX;//x成分の相対座標
		int delY = Player.posY - eBullet.posY;//y成分の相対座標
		int delR = Player.R + eBullet.R;//それぞれの半径を足す
		//円の当たり判定
		// お互いの中心距離が、お互いの半径を足した距離より小さくなっていれば当たっている。
		// 三平方の定理から二乗を外して以下の式を作る。
		if ((delX*delX + delY*delY) < (delR*delR))
		{
			isPlayerHit = true;
		}
		else
		{
			isPlayerHit = false;
		}
		//Enemyの当たり判定
		delX = Enemy.posX - pBullet.posX;
		delY = Enemy.posY - pBullet.posY;
		delR = Enemy.R + pBullet.R;
		if ((delX * delX + delY * delY) < (delR * delR))
		{
			isEnemyHit = true;
		}
		else
		{
			isEnemyHit = false;
		}


		/*Draw*/
		//Player.
		DrawRotaGraph(Player.posX, Player.posY, Player.scale, Player.rotate, Player.drawHandle, 1);
		//PlayerBullet.
		if (isPlayerBullet)
		{
			DrawCircle(pBullet.posX, pBullet.posY, pBullet.R, pBullet.color, 1);
		}
		//Enemy.
		if (isEnemyHit)
		{
			DrawCircle(Enemy.posX, Enemy.posY, Enemy.R, Enemy.hitColor, 1);
		}
		else
		{
			DrawCircle(Enemy.posX, Enemy.posY, Enemy.R, Enemy.color, 1);
		}
		//EnemyBullet.
		if (isEnemyBullet)
		{
			DrawCircle(eBullet.posX, eBullet.posY, eBullet.R, eBullet.color, 1);
		}


		/*DebugDraw*/
		DrawString(0, 0, "操作説明:WASD(上左下右),Enter(発射)", GetColor(255, 0, 0));
		DrawFormatString(0,20, GetColor(255, 0, 0),"Playerの当たり判定:%d", isPlayerHit);

		//裏画面を表へ
		ScreenFlip();

		//リフレッシュ処理(-1ならエラー)
		if (ProcessMessage() < 0) { break; }

		//ループ終了処理
		if (CheckHitKey(KEY_INPUT_ESCAPE)) { break; }//escで強制終了

		//fps固定(60fps:16.66ms)
		//ループ開始時刻から16.66ms経つまで停止
		while (GetNowHiPerformanceCount() - roopStartTime < 16667) {}
	}

	/*終了処理*/
	DxLib_End();//Dxlib終了処理
	return 0;//終了 
}