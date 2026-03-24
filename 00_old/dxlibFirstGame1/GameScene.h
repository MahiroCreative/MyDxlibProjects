#pragma once
#include "GameCommon.h"
#include "DxLib.h"
#include "MyGameLib.h"
#include "RankingScene.h"


class GameScene
{
private:
	/*メンバ変数*/
	SceneKind _nextScene;//シーン更新用
	IntVector _charaPos;//キャラクタの位置
	IntVector _charaColPos;//キャラクタのコリジョンの位置
	IntVector _enemyPos;//エネミーの位置
	IntVector _shotPos;//ショットの位置
	IntVector _bulletPos;//弾丸の位置
	IntVector _bullet2Pos;//弾丸の位置
	IntVector _bullet3Pos;//弾丸の位置
	IntVector _bullet4Pos;//弾丸の位置
	IntVector _bullet5Pos;//弾丸の位置
	int _charaHandle;//キャラの画像ハンドル
	int _charaSpeed;//キャラの移動速度
	int _charaColR;//キャラコリジョンの半径
	int _enemyR;//エネミーの半径
	int _bulletColR;//弾丸コリジョンの半径
	int _debugInt;//Debug用
	int _bulletSpeed;
	int _shotR;
	int _debug;
	int _score;
	bool _isHitFlag;//当たり判定フラグ
	bool _isEnemyHitFlag;
	bool _isShotFlag;
public:
	/*コンストラクタ*/
	GameScene() :
		_nextScene(SceneKind::GAMESCENE),
		_charaPos(80, 360),
		_charaColPos(_charaPos.X + 23, _charaPos.Y + 20),
		_charaColR(16),
		_charaHandle(-1),
		_charaSpeed(4),
		_enemyPos(1280,360),
		_enemyR(240),
		_shotPos(_charaPos.X, _charaPos.Y + 20),
		_bulletPos(800, 120),
		_bullet2Pos(800, 240),
		_bullet3Pos(800, 360),
		_bullet4Pos(800, 480),
		_bullet5Pos(800, 600),
		_bulletColR(8),
		_bulletSpeed(4),
		_shotR(8),
		_debug(0),
		_score(0),
		_isHitFlag(false),
		_isEnemyHitFlag(false),
		_isShotFlag(false)
	{
		_charaHandle = LoadGraph("Chara.png");
	};
	/*メンバ関数*/
	//処理の更新
	SceneKind Update()
	{

		/*キャラ更新*/
		//移動操作
		//up.
		if (MyKeyInput::isHoldKey(KEY_INPUT_W) && _charaPos.Y > 0)
		{
			_charaPos.Y -= _charaSpeed;
		}
		//down.
		if (MyKeyInput::isHoldKey(KEY_INPUT_S) && _charaPos.Y < 680)
		{
			_charaPos.Y += _charaSpeed;
		}
		//left.
		if (MyKeyInput::isHoldKey(KEY_INPUT_A))
		{
			_charaPos.X -= _charaSpeed;
		}
		//right.
		if (MyKeyInput::isHoldKey(KEY_INPUT_D))
		{
			_charaPos.X += _charaSpeed;
		}
		//コリジョン位置の更新
		_charaColPos.X = _charaPos.X + 23;
		_charaColPos.Y = _charaPos.Y + 20;

		/*弾丸更新*/
		//弾丸1:移動
		_bulletPos.X -= _bulletSpeed;
		//弾丸2:移動
		_bullet2Pos.X -= _bulletSpeed;
		//弾丸3:移動
		_bullet3Pos.X -= _bulletSpeed;
		//弾丸4:移動
		_bullet4Pos.X -= _bulletSpeed;
		//弾丸5:移動
		_bullet5Pos.X -= _bulletSpeed;

		/*弾丸の初期化*/
		if (_bulletPos.X < 0)
		{
			//1.
			_bulletPos.X = 1100;
			_bulletPos.Y = rand() % 240;
			//2.
			_bullet2Pos.X = 1100;
			_bullet2Pos.Y = rand() % 120 + 120;
			//3
			_bullet3Pos.X = 1100;
			_bullet3Pos.Y = rand() % 240 + 240;
			//4.
			_bullet4Pos.X = 1100;
			_bullet4Pos.Y = rand() % 360 + 360;
			//5.
			_bullet5Pos.X = 1100;
			_bullet5Pos.Y = rand() % 240 + 480;
			//弾丸速度更新
			_bulletSpeed += 2;
		}

		/*キャラ当たり判定*/
		//弾丸1との当たり判定計算
		int HitLength = _charaColR + _bulletColR;//共通部
		IntVector delVec = _charaColPos - _bulletPos;
		int delLength = delVec.Length();
		//弾丸2との当たり判定計算
		IntVector delVec2 = _charaColPos - _bullet2Pos;
		int delLength2 = delVec2.Length();
		//弾丸3との当たり判定計算
		IntVector delVec3 = _charaColPos - _bullet3Pos;
		int delLength3 = delVec3.Length();
		//弾丸4との当たり判定計算
		IntVector delVec4 = _charaColPos - _bullet4Pos;
		int delLength4 = delVec4.Length();
		//弾丸5との当たり判定計算
		IntVector delVec5 = _charaColPos - _bullet5Pos;
		int delLength5 = delVec5.Length();
		//判定
		if (HitLength > delLength)
		{
			_isHitFlag = true;
		}
		else if (HitLength > delLength2)//弾丸２
		{
			_isHitFlag = true;
		}
		else if (HitLength > delLength3)//弾丸３
		{
			_isHitFlag = true;
		}
		else if (HitLength > delLength4)//弾丸４
		{
			_isHitFlag = true;
		}
		else if (HitLength > delLength5)//弾丸5
		{
			_isHitFlag = true;
		}
		else
		{
			_isHitFlag = false;
		}

		/*ショット*/
		if (MyMouseInput::isHoldKey(MOUSE_INPUT_LEFT))//クリック中は溜める
		{
			_isShotFlag = true;
		}
		//移動
		if (_shotPos.X > 1280)
		{
			_shotPos = _charaPos;
			_isShotFlag = false;
		}
		else if(_isShotFlag)
		{
			_shotPos.X += 16;
		}

		/*ショットと敵の当たり判定*/
		int HitLengthEnemy = _shotR + _enemyR;
		IntVector delVecEnemy = _shotPos - _enemyPos;
		int delEnemyLength = delVecEnemy.Length();
		if (HitLengthEnemy > delEnemyLength)
		{
			_isEnemyHitFlag = true;
			_score++;
		}
		else
		{
			_isEnemyHitFlag = false;
		}

		/*シーン遷移処理*/
		//Enterキーを押したらTitleシーンに遷移
		if (MyKeyInput::isDownKey(KEY_INPUT_RETURN))
		{
			_nextScene = SceneKind::TITLESCENE;
		}
		//敵と当たっていたらタイトル画面へ。
		if (_isHitFlag)
		{
			if (RankingScene::_maxScore < _score)
			{
				RankingScene::_maxScore = _score;
			}

			_nextScene = SceneKind::TITLESCENE;
		}

		return _nextScene;
	}
	//描画更新
	void Draw()
	{
		//キャラクターの描画
		DrawGraph(_charaPos.X, _charaPos.Y, _charaHandle, TRUE);

		//エネミーの描画
		if (_isEnemyHitFlag)
		{
			DrawCircle(_enemyPos.X, _enemyPos.Y, _enemyR, GetColor(0, 155, 155), 1);

		}
		else
		{
			DrawCircle(_enemyPos.X, _enemyPos.Y, _enemyR, GetColor(255, 255, 255), 1);
		}


		//弾丸の描画
		DrawCircle(_bulletPos.X, _bulletPos.Y, 8, GetColor(255, 0, 0), 1);
		DrawCircle(_bullet2Pos.X, _bullet2Pos.Y, 8, GetColor(255, 0, 0), 1);
		DrawCircle(_bullet3Pos.X, _bullet3Pos.Y, 8, GetColor(255, 0, 0), 1);
		DrawCircle(_bullet4Pos.X, _bullet4Pos.Y, 8, GetColor(255, 0, 0), 1);
		DrawCircle(_bullet5Pos.X, _bullet5Pos.Y, 8, GetColor(255, 0, 0), 1);

		//ショットの描画
		if (_isShotFlag)
		{
			DrawCircle(_shotPos.X, _shotPos.Y, 8, GetColor(0, 0, 255), 1);
		}

	}
	//デバッグ用の描画更新
	void DebugDraw()
	{
		//現在のシーン名を描画
		DrawString(8, 8, "SceneName:GameScene", GetColor(0, 255, 255));

		//キャラクターのコリジョンの描画
		DrawCircle(_charaColPos.X, _charaColPos.Y, _charaColR, GetColor(0, 255, 255), 0);

		//弾丸のコリジョンの描画
		DrawCircle(_bulletPos.X, _bulletPos.Y, _bulletColR, GetColor(0, 255, 255), 0);
		DrawCircle(_bullet2Pos.X, _bullet2Pos.Y, _bulletColR, GetColor(0, 255, 255), 0);
		DrawCircle(_bullet3Pos.X, _bullet3Pos.Y, _bulletColR, GetColor(0, 255, 255), 0);
		DrawCircle(_bullet4Pos.X, _bullet4Pos.Y, _bulletColR, GetColor(0, 255, 255), 0);
		DrawCircle(_bullet5Pos.X, _bullet5Pos.Y, _bulletColR, GetColor(0, 255, 255), 0);

		//DebugNum
		DrawFormatString(10, 700, GetColor(0, 255, 255), "Score:%d", _score);
	}
	/// <summary>
	/// インスタンスの初期化
	/// </summary>
	void Init()
	{
		//メンバ変数の初期化
		_nextScene = SceneKind::GAMESCENE;

		//弾丸初期化
		//1.
		_bulletPos.X = 1100;
		_bulletPos.Y = 120;
		//2.
		_bullet2Pos.X = 1100;
		_bullet2Pos.Y = 240;
		//3
		_bullet3Pos.X = 1100;
		_bullet3Pos.Y = 360;
		//4.
		_bullet4Pos.X = 1100;
		_bullet4Pos.Y = 480;
		//5.
		_bullet5Pos.X = 1100;
		_bullet5Pos.Y = 600;

		//弾丸速度の初期化
		_bulletSpeed = 4;

		//score初期化
		_score = 0;
	}
};