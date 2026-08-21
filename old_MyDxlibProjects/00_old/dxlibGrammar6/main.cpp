#include "DxLib.h"

/*概要*/
//この『Grammerシリーズ』ではDxLibやゲームプログラミングの基礎的な使い方と考え方を段階的にやります。
//解説する内容は必要最低限です。抜けが出ます。また詳細な解説をしていません。
//随時リファレンスや外部サイトを参考にしてください。
//【リファレンス】
// https://dxlib.xsrv.jp/dxfunc.html

/*今回の要素*/
//・ゲームプログラミング
// 　- ポインタを積極的に利用して可能な限り関数化
//・C/C++言語
//   - ポインタ

/*コメント*/
//・関数型の処理(ポインタ含む)
//・オブジェクト指向処理
// の順で理解してほしくて、このように長々と回りくどくやってきました。
//ここまでやったら関数型の処理はもう大丈夫でしょう。
//次回からはオブジェクト指向になります。

/*構造体の作成*/
//GameOject.
struct GameObject
{
	int DrawHandle = NULL;//画像ハンドル
	int X = NULL;//X座標
	int Y = NULL;//Y座標
	int R = NULL;//半径
	int Speed = NULL;//速度
	int MaxShotNum = NULL;//発射できる最大弾数
	int ShotCount = NULL;//現在発射している弾数
	unsigned int Color = NULL;//色
	unsigned int HitColor = NULL;//Hit色
	double Scale = NULL;//スケール
	double Rotate = NULL;//回転度
	bool IsHit = NULL;//当たり判定
	bool IsVisible = NULL;//表示されているか
};
//KeyInput
struct IsKeyInput
{
	bool IsNow = false;//押された瞬間True
	bool IsHold = false;//押されている最中True
};

/*プロトタイプ宣言*/
//オブジェクト系.
//ポインタでアドレスを受け取ることで、グローバルでなくとも変更可能
//また、配列はポインタなので(使いやすくなっている)、ポインタとして渡せる。
void PlayerUpdate(GameObject* _pPlayer,GameObject* _pPBullet,IsKeyInput _keyFlag);
void EnemyUpdate(GameObject* _pEnemy, GameObject* _pEBullet);
void PlayerBulletUpdate(GameObject* _pPlayer, GameObject* _pPBullet);
void EnemyBuletUpdate(GameObject* _pEnemy,GameObject* _pEBullet);
//機能系.
bool IsHitCollision(GameObject _hitObj, GameObject _bullets[], int MAX);//第一引数と第二引数の当たり判定.第三引数は配列の最大数
IsKeyInput InputKeyUpdate(IsKeyInput _keyFlag);//第一引数でキーフラグを渡し、更新を引数で得る
void Draw(GameObject _gameObjects[], int _objNum);//画面表示
void DebugDraw(GameObject _gameObjects[], int _objNum);//Debug表示

//Dxlibのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	/*ゲームループ用変数*/
	LONGLONG roopStartTime = 0;
	bool gameRoop = true;

	/*Dxlib初期化*/
	SetGraphMode(1280, 720, 32);//画面サイズと解像度
	ChangeWindowMode(true);//Windowモード
	if (DxLib_Init() == -1) { return -1; }//Dxlib初期化
	SetDrawScreen(DX_SCREEN_BACK);//ダブルバッファリング

	/*ゲーム用定数*/
	constexpr int MAX_SHOT_NUM = 3;
	constexpr int MAX_GAMEOBJ_NUM = MAX_SHOT_NUM * 2 + 2;

	/*ゲーム用変数*/
	//InputKey.
	IsKeyInput IsKeyFlag;
	//Player.
	GameObject Player;
	Player.DrawHandle = LoadGraph("Chara.png");
	Player.X = 100;
	Player.Y = 300;
	Player.R = 12;
	Player.Scale = 1;
	Player.MaxShotNum = MAX_SHOT_NUM;
	Player.ShotCount = 0;
	Player.Rotate = 0;
	Player.Speed = 4;
	Player.IsHit = false;
	Player.IsVisible = true;
	//Enemy.
	GameObject Enemy;
	Enemy.X = 1000;
	Enemy.Y = 360;
	Enemy.R = 80;
	Enemy.Speed = 1;
	Enemy.MaxShotNum = MAX_SHOT_NUM;
	Enemy.ShotCount = 0;
	Enemy.Color = GetColor(255, 0, 255);
	Enemy.HitColor = GetColor(0, 0, 255);
	Enemy.IsHit = false;
	Enemy.IsVisible = true;
	//PlayerBullet.
	GameObject pBullet[MAX_SHOT_NUM];
	for (int i = 0; i < MAX_SHOT_NUM; i++)//初期化
	{
		pBullet[i].X = Player.X;
		pBullet[i].Y = Player.Y;
		pBullet[i].R = 4;
		pBullet[i].Speed = 8;
		pBullet[i].Color = GetColor(255, 255, 255);
		pBullet[i].IsVisible = false;
	}
	//EnemyBullet.
	GameObject eBullet[MAX_SHOT_NUM];
	for (int i = 0; i < MAX_SHOT_NUM; i++)//初期化
	{
		eBullet[i].X = Enemy.X;
		eBullet[i].Y = Enemy.Y;
		eBullet[i].R = 32;
		eBullet[i].Speed = 8;
		eBullet[i].Color = GetColor(0, 255, 0);
		eBullet[i].IsVisible = false;
	}
	//DrawOjects.
	GameObject drawObjects[MAX_GAMEOBJ_NUM];

	/*ゲームループ部*/
	//gameRoop.
	while (gameRoop)
	{
		//ループ開始時刻の確保
		roopStartTime = GetNowHiPerformanceCount();

		//裏画面の初期化
		ClearDrawScreen();

		/*Key入力の更新*/
		IsKeyFlag = InputKeyUpdate(IsKeyFlag);

		/*Player処理*/
		PlayerUpdate(&Player,pBullet,IsKeyFlag);//参照渡しをすることで、アドレスを引数として渡せる。

		/*PlayerBulletの処理*/
		PlayerBulletUpdate(&Player,pBullet);

		/*Enemy処理*/
		EnemyUpdate(&Enemy, eBullet);

		/*EnemyBulletの処理*/
		EnemyBuletUpdate(&Enemy,eBullet);

		/*当たり判定*/
		Player.IsHit = IsHitCollision(Player, eBullet, MAX_SHOT_NUM);//Playerの当たり判定
		Enemy.IsHit = IsHitCollision(Enemy, pBullet, MAX_SHOT_NUM);//Enemyの当たり判定

		/*GameObjectを一つの配列にする*/
		//Playerの挿入
		drawObjects[0] = Player;
		//Enemyの挿入
		drawObjects[1] = Enemy;
		//PlayerBulletの挿入
		for (int i = 0; i < MAX_SHOT_NUM; i++)
		{
			drawObjects[i + 2] = pBullet[i];
		}
		//EnemyBulletの挿入
		for (int i = 0; i < MAX_SHOT_NUM; i++)
		{
			drawObjects[i + 2 + MAX_SHOT_NUM] = eBullet[i];
		}

		/*Draw*/
		Draw(drawObjects, MAX_GAMEOBJ_NUM);

		/*DebugDraw*/
		DebugDraw(drawObjects, MAX_GAMEOBJ_NUM);

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


/*オブジェクト系関数*/
/// <summary>
/// プレイヤー処理の更新
/// </summary>
/// <param name="_pPlayer">プレイヤーのポインタ</param>
/// <param name="_pPBullet">ポインタとして配列を渡す</param>
/// <param name="_keyFlag">キーフラグ</param>
void PlayerUpdate(GameObject* _pPlayer, GameObject* _pPBullet,IsKeyInput _keyFlag)
{
	//移動処理
	if (CheckHitKey(KEY_INPUT_W))//Wで上移動
	{
		_pPlayer->Y -= _pPlayer->Speed;
	}
	else if (CheckHitKey(KEY_INPUT_S))//Sで下移動
	{
		_pPlayer->Y += _pPlayer->Speed;
	}
	else if (CheckHitKey(KEY_INPUT_D))//Dで右移動
	{
		_pPlayer->X += _pPlayer->Speed;
	}
	else if (CheckHitKey(KEY_INPUT_A))//Aで左移動
	{
		_pPlayer->X -= _pPlayer->Speed;
	}
	//弾の発射
	if (_keyFlag.IsNow && _pPlayer->ShotCount < _pPlayer->MaxShotNum)
	{
		//現在撃っている球数の更新
		_pPlayer->ShotCount++;
		for (int i = 0; i < _pPlayer->ShotCount; i++)
		{
			_pPBullet[i].IsVisible = true;
		}
	}
}
/// <summary>
/// Playerの弾丸の更新
/// </summary>
/// <param name="_pPlayer">プレイヤーのポインタ</param>
/// <param name="_pPBullet">ポインタとして配列を渡す</param>
void PlayerBulletUpdate(GameObject* _pPlayer,GameObject* _pPBullet)
{
	/*PlayerBulletの処理*/
	for (int i = 0; i < _pPlayer->MaxShotNum; i++)
	{
		if (_pPBullet[i].IsVisible && _pPBullet[i].X > 1280)//画面外に出たなら初期化
		{
			//初期化
			_pPBullet[i].X = _pPlayer->X;
			_pPBullet[i].Y = _pPlayer->Y;
			_pPBullet[i].IsVisible = false;
			//初期化された数だけ撃っている球数を減らす
			_pPlayer->ShotCount--;
		}
		else if (_pPBullet[i].IsVisible)//画面内なら移動
		{
			//移動処理
			_pPBullet[i].X += _pPBullet[i].Speed;
		}
		else//見えてない状態でもプレイヤーに追随
		{
			_pPBullet[i].X = _pPlayer->X;
			_pPBullet[i].Y = _pPlayer->Y;
		}
	}
}
/// <summary>
/// エネミー処理の更新
/// </summary>
/// <param name="_pEnemy">エネミーのポインタ</param>
/// <param name="_pEBullet">ポインタとして配列を渡す</param>
void EnemyUpdate(GameObject* _pEnemy, GameObject* _pEBullet)
{
	//移動処理
	_pEnemy->Y += _pEnemy->Speed;
	if (_pEnemy->Y > 640 || _pEnemy->Y < 60)//上下運動をさせている。
	{
		_pEnemy->Speed = -_pEnemy->Speed;
	}
	//変則的に発射
	if (_pEnemy->ShotCount < _pEnemy->MaxShotNum)//最大玉数に達していない。
	{
		if (_pEnemy->Y % 80 == 0)//かつ、Y座標が80で割り切れるとき
		{
			_pEBullet[_pEnemy->ShotCount].IsVisible = true;
			_pEnemy->ShotCount++;
		}
	}
}
/// <summary>
/// Playerの弾丸の更新
/// </summary>
/// <param name="_pEnemy">エネミーのポインタ</param>
/// <param name="_pEBullet">ポインタとして配列を渡す</param>
void EnemyBuletUpdate(GameObject* _pEnemy, GameObject* _pEBullet)
{
	for (int i = 0; i < _pEnemy->MaxShotNum; i++)
	{
		if (_pEBullet[i].IsVisible && _pEBullet[i].X < 0)//画面外に出たなら初期化
		{
			//初期化
			_pEBullet[i].X = _pEnemy->X;
			_pEBullet[i].Y = _pEnemy->Y;
			_pEBullet[i].IsVisible = false;
			//撃っている弾丸数を初期化
			_pEnemy->ShotCount = 0;
		}
		else if (_pEBullet[i].IsVisible)//画面内なら移動
		{
			//移動処理
			_pEBullet[i].X -= _pEBullet[i].Speed;
		}
		else//見えてない状態でもエネミーに追随
		{
			_pEBullet[i].X = _pEnemy->X;
			_pEBullet[i].Y = _pEnemy->Y;
		}
	}
}



/*機能系関数*/
/// <summary>
/// キーの状態をを取得.押された瞬間か、または押され続けているのかが返される。
/// ChceckHitKey()では押されている間もずっと反応するので、押した瞬間を取得できない(一回だけ押したつもりでも10回くらい押された扱いになる)。
/// そのため、この関数で『押された瞬間』を取得する。今回はプレイヤーの弾の発射で使用する。
/// 第一引数でキーフラグを渡し、更新を引数で得る.
/// ゲームループ中に毎フレーム実行することで確認している。
/// 今回はエンターだけをチェックしている。
/// </summary>
/// <param name="_keyFlag">自身で作成したキーフラグ</param>
/// <returns></returns>
IsKeyInput InputKeyUpdate(IsKeyInput _keyFlag)
{
	IsKeyInput ans;

	//EnterKey.
	if (CheckHitKey(KEY_INPUT_RETURN) && !_keyFlag.IsHold)//エンターが押され、押され続けていない。
	{
		//押された瞬間
		ans.IsNow = true;
		ans.IsHold = true;
	}
	else if (CheckHitKey(KEY_INPUT_RETURN) && _keyFlag.IsHold)//エンターが押され、押され続けている.
	{
		//押され続けている
		ans.IsNow = false;
		ans.IsHold = true;
	}
	else//Enterが押されていない。
	{
		ans.IsNow = false;
		ans.IsHold = false;
	}

	return ans;
}
/// <summary>
/// 第一引数と第二引数の当たり判定
/// </summary>
/// <param name="_hitObj"></param>
/// <param name="_bullets"></param>
/// <returns></returns>
bool IsHitCollision(GameObject _hitObj, GameObject _bullets[], int MAX)
{
	int delX;//X成分の距離
	int delY;//Y成分の距離
	int delR;//半径の合計

	for (int i = 0; i < MAX; i++)
	{
		//計算
		delX = _hitObj.X - _bullets[i].X;
		delY = _hitObj.Y - _bullets[i].Y;
		delR = _hitObj.R + _bullets[i].R;
		//判定
		if ((delX * delX + delY * delY) < (delR * delR))
		{
			return true;
		}
	}
	return false;
}
/// <summary>
/// Dxlibを用いて画面表示を行う関数。
/// </summary>
/// <param name="_gameObjects">全オブジェクトの配列</param>
/// <param name="_objNum">ゲームオブジェクトの総数</param>
void Draw(GameObject _gameObjects[], int _objNum)
{
	for (int i = 0; i < _objNum; i++)
	{
		if ((_gameObjects[i].DrawHandle != NULL) && _gameObjects[i].IsVisible)//画像ハンドル有り
		{
			//Plyerの描画
			DrawRotaGraph(_gameObjects[i].X, _gameObjects[i].Y, _gameObjects[i].Scale, _gameObjects[i].Rotate, _gameObjects[i].DrawHandle, 1);
		}
		else if (_gameObjects[i].IsVisible)//画像ハンドル無し
		{
			//Plyer以外の描画
			if (_gameObjects[i].IsHit)//当たった時。
			{
				DrawCircle(_gameObjects[i].X, _gameObjects[i].Y, _gameObjects[i].R, _gameObjects[i].HitColor);
			}
			else
			{
				DrawCircle(_gameObjects[i].X, _gameObjects[i].Y, _gameObjects[i].R, _gameObjects[i].Color);
			}
		}
	}
}
/// <summary>
/// Dxlibを用いて画面表示を行う関数(Debug用)。
/// </summary>
/// <param name="_gameObjects">オブジェクト全ての配列</param>
/// <param name="_objNum">ゲームオブジェクトの総数</param>
void DebugDraw(GameObject _gameObjects[], int _objNum)
{
	//オブジェクト毎
	for (int i = 0; i < _objNum; i++)
	{
		if (_gameObjects[i].IsVisible)
		{
			DrawCircle(_gameObjects[i].X, _gameObjects[i].Y, _gameObjects[i].R, GetColor(255, 0, 0), 0);
		}

		/*デバッグテキスト*/
		if ((_gameObjects[i].DrawHandle != NULL) && _gameObjects[i].IsVisible)//画像ハンドル有り(Player)
		{
			//Plyerの描画
			DrawFormatString(0, 20, GetColor(255, 0, 0), "PlayerHit:%d", _gameObjects[i].IsHit);
		}
	}

	/*操作説明*/
	DrawString(0, 0, "操作説明:WASD(上左下右),Enter(発射)", GetColor(255, 0, 0));
}