#include "DxLib.h"

/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//解説する内容は必要最低限です。抜けが出ます。また詳細な解説をしていません。
//随時リファレンスや外部サイトを参考にしてください。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・ゲームプログラミング
// 　- 複数オブジェクトの制御
//   - 類似処理の関数化
//   - コリジョンのデバッグ表示
//・C/C++言語
//   - グローバル変数
//   - 構造体の初期化
// 　- 定数(constexpr)
// 　- Roop文(for文)
//   - 配列
//   - 関数

/*構造体の作成(グローバル変数として構造体を作成)*/
//構造体をグローバル変数として作成する。Mainの外に記述すればよい。
//グローバル変数として作成することで、Main以外のメソッドでも使用することができる。
//また、作成時点で初期化しておくことも可能.
//Player.
struct Player
{
	int DrawHandle = LoadGraph("Chara.png");//画像ハンドル
	int X = 100;//X座標
	int Y = 300;//Y座標
	int R = 12;//半径
	double Scale = 1;//スケール
	double Rotate = 0;//回転度
	int Speed = 4;//速度
	bool IsShot = false;//現在弾を発射しているか
	bool IsHit = false;//当たり判定用のフラグ
};
//Enemy.
struct Enemy
{
	int X = 1000;//X座標
	int Y = 360;//Y座標
	int R = 80;//半径
	int Speed = 1;//速度
	int MaxShotNum = NULL;//発射できる最大弾丸数
	int ShotCount = 0;//現在発射している弾丸数
	unsigned int Color = GetColor(255, 0, 255);
	unsigned int hitColor = GetColor(0, 0, 255);
	bool IsHit = false;//当たり判定用のフラグ
};
//Bullet.
struct Bullet
{
	int X = NULL;//X座標
	int Y = NULL;//Y座標
	int R = NULL;//半径
	int Speed = NULL;//速度
	unsigned int Color = NULL;//速度
	bool IsVisible = false;
};

/*関数*/
//類似機能は関数としてまとめてしまう。
//プロトタイプ宣言しておくことで、後から処理を記述しても使えるようになる。
//プロトタイプ宣言.
bool IsHitPlayerCollision(Player _hitObj, Bullet _eBullets[], int _MAXSHOT);//Playerとエネミーの弾が当たったかを判定
bool IsHitEnemyCollision(Enemy _hitObj, Bullet _pBullet);//エネミーとプレイヤーの弾が当たったかを判定
void Draw(Player _player, Enemy _enemy, Bullet _pBullet, Bullet _eBullets[]);//画面表示
void DebugDraw(Player _player, Enemy _enemy, Bullet _pBullet, Bullet _eBullets[]);//Debug表示

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	/*定数*/
	//プログラムの実行後に変更する可能性が無い数値は、定数として確保すべきである。
	// 定数の作り方は、『#define』や『const』など色々あるが、
	// 単なる定数として扱うなら『constexpr』が一番問題を起こさない可能性が高い。
	// その他の定数は必要になった際に学習して使用すればよい。
	//画面サイズ(定数).
	constexpr int SCREEN_SIZE_X = 1280;//幅
	constexpr int SCREEN_SIZE_Y = 720;//高さ
	//エネミーの最大弾丸数
	constexpr int MAX_ENEMY_SHOT = 3;

	/*変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*変数*/
	//Player.
	Player Player;
	//Enemy.
	Enemy Enemy;
	Enemy.MaxShotNum = MAX_ENEMY_SHOT;
	//PlayerBullet.
	Bullet pBullet;
	pBullet.X = Player.X;
	pBullet.Y = Player.Y;
	pBullet.R = 4;
	pBullet.Speed = 8;
	pBullet.Color = GetColor(255, 255, 255);
	//EnemyBullet.
	//同じものを一気に扱うなら配列
	Bullet eBullets[MAX_ENEMY_SHOT];

	/*EnemyBulletの初期化処理*/
	//配列にしておけば、ループ分などで一気に初期化可能
	// (if文などと連携すれば条件に応じて値も変更できる)
	for (int i = 0; i < MAX_ENEMY_SHOT; i++)
	{
		eBullets[i].X = Enemy.X;
		eBullets[i].Y = Enemy.Y;
		eBullets[i].R = 32;
		eBullets[i].Speed = 8;
		eBullets[i].Color = GetColor(0, 255, 0);
	}

	/*ゲームループ部*/
	//gameRoop.
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
			Player.Y -= Player.Speed;
		}
		else if (CheckHitKey(KEY_INPUT_S))//Sで下移動
		{
			Player.Y += Player.Speed;
		}
		else if (CheckHitKey(KEY_INPUT_D))//Dで右移動
		{
			Player.X += Player.Speed;
		}
		else if (CheckHitKey(KEY_INPUT_A))//Aで左移動
		{
			Player.X -= Player.Speed;
		}
		//弾の発射
		if (CheckHitKey(KEY_INPUT_RETURN) && !Player.IsShot)//弾は一発しか発射できない
		{
			Player.IsShot = true;
		}

		/*Enemy処理*/
		//移動処理
		Enemy.Y += Enemy.Speed;
		if (Enemy.Y > 640 || Enemy.Y < 60)//上下運動をさせている。
		{
			Enemy.Speed = -Enemy.Speed;
		}
		//変則的に発射
		if (Enemy.ShotCount < Enemy.MaxShotNum)//最大玉数に達していない。
		{
			if (Enemy.Y % 80 == 0)//かつ、Y座標が80で割り切れるとき
			{
				eBullets[Enemy.ShotCount].IsVisible = true;
				Enemy.ShotCount++;
			}
		}

		/*PlayerBullet処理*/
		if (Player.IsShot && pBullet.X < 1280)//弾が発射されており、画面内なら移動
		{
			pBullet.X += pBullet.Speed;
		}
		else//画面外なら初期化
		{
			pBullet.X = Player.X;
			pBullet.Y = Player.Y;
			Player.IsShot = false;
		}

		/*EnemyBulletの処理*/
		for (int i = 0; i < MAX_ENEMY_SHOT; i++)
		{
			if (eBullets[i].IsVisible && eBullets[i].X < 0)//画面外に出たなら初期化
			{
				//初期化
				eBullets[i].X = Enemy.X;
				eBullets[i].Y = Enemy.Y;
				eBullets[i].IsVisible = false;
				//撃っている弾丸数を初期化
				Enemy.ShotCount = 0;
			}
			else if (eBullets[i].IsVisible)//画面内なら移動
			{
				//移動処理
				eBullets[i].X -= eBullets[i].Speed;
			}
			else//見えてない状態でもエネミーに追随
			{
				eBullets[i].X = Enemy.X;
				eBullets[i].Y = Enemy.Y;
			}
		}


		/*当たり判定*/
		//Enemyの当たり判定.
		Enemy.IsHit = IsHitEnemyCollision(Enemy, pBullet);
		//Playerの当たり判定
		Player.IsHit = IsHitPlayerCollision(Player, eBullets, MAX_ENEMY_SHOT);


		/*Draw*/
		Draw(Player, Enemy, pBullet, eBullets);

		/*DebugDraw*/
		DebugDraw(Player, Enemy, pBullet, eBullets);

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



/*関数の実装*/
//下記のような書き方をすると、関数にマウスポインタを重ねた際にコメントを参照できる。

/// <summary>
/// Playerとエネミーの弾が当たったかを判定
/// </summary>
bool IsHitPlayerCollision(Player _hitObj, Bullet _eBullets[], int MAXSHOT)
{
	int delX;//X成分の距離
	int delY;//Y成分の距離
	int delR;//半径の合計

	for (int i=0; i < MAXSHOT; i++)
	{
		//計算
		delX = _hitObj.X - _eBullets[i].X;
		delY = _hitObj.Y - _eBullets[i].Y;
		delR = _hitObj.R + _eBullets[i].R;
		//判定
		if ((delX * delX + delY * delY) < (delR * delR))
		{
			return true;
		}
	}

	return false;
}

/// <summary>
/// エネミーとプレイヤーの弾が当たったかを判定
/// </summary>
bool IsHitEnemyCollision(Enemy _hitObj, Bullet _pBullet)
{
	bool ans = false;
	int delX = _hitObj.X - _pBullet.X;//X成分の距離
	int delY = _hitObj.Y - _pBullet.Y;//Y成分の距離
	int delR = _hitObj.R + _pBullet.R;//半径の合計

	if ((delX * delX + delY * delY) < (delR * delR))
	{
		ans = true;
	}
	else
	{
		ans = false;
	}

	return ans;
}

/// <summary>
/// Dxlibを用いて画面表示を行う関数。
/// </summary>
void Draw(Player _player, Enemy _enemy, Bullet _pBullet, Bullet _eBullets[])
{
	//Plyerの描画
	DrawRotaGraph(_player.X, _player.Y, _player.Scale, _player.Rotate, _player.DrawHandle, 1);
	//Enemyの描画
	if (_enemy.IsHit)//当たってるとき
	{
		DrawCircle(_enemy.X, _enemy.Y, _enemy.R, _enemy.hitColor);
	}
	else//当たってないとき
	{
		DrawCircle(_enemy.X, _enemy.Y, _enemy.R, _enemy.Color);
	}

	//PlayerBulletの描画
	if (_player.IsShot)
	{
		DrawCircle(_pBullet.X, _pBullet.Y, _pBullet.R, _pBullet.Color);
	}
	//EnemyBulletの描画
	for (int i = 0; i < _enemy.MaxShotNum; i++)
	{
		if (_eBullets[i].IsVisible)
		{
			DrawCircle(_eBullets[i].X, _eBullets[i].Y, _eBullets[i].R, _eBullets[i].Color, 1);
		}

	}
}

/// <summary>
/// Dxlibを用いて画面表示を行う関数(Debug用)。
/// </summary>
void DebugDraw(Player _player, Enemy _enemy, Bullet _pBullet, Bullet _eBullets[])
{
	/*操作説明*/
	DrawString(0, 0, "操作説明:WASD(上左下右),Enter(発射)", GetColor(255, 0, 0));
	/*コリジョン表示*/
	//コリジョン(当たり判定の範囲)を表示することでデバッグしやくする。
	//Player.
	DrawCircle(_player.X, _player.Y, _player.R, GetColor(255, 0, 0), 0);
	//デバッグテキスト
	DrawFormatString(0, 20, GetColor(255, 0, 0), "PlayerHit:%d", _player.IsHit);
}