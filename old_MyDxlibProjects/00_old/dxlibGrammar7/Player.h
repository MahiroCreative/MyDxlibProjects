#pragma once
#include "Enemy.h"
#include "MyDxlibCommon.h"
#include "DxLib.h"

class Player
{
public:
	/*publicメンバ変数*/
	int X = NULL;//x座標
	int Y = NULL;//y座標
	/*コンストラクタ*/
	Player();
	/*Init*/
	void Init(int drawHandle, int x, int y, int r, int speed, bool visible, bool colVisible);
	/*Getter*/
	bool getShotFlag();
	int getR();
	/*メンバ関数*/
	void Update();//処理の更新
	void Draw();//表示の更新
	void DebugDraw();//Debug表示の更新
private:
	/*privateメンバ変数*/
	int _drawHandle = -1;//画像ハンドル
	int _r = NULL;//半径
	int _speed = NULL;//スピード
	//変更される可能性の無いものはここで初期化
	unsigned int _colColer = ColorCode::LIME;//コリジョン色
	int _scale = 1;//スケール
	int _rotate = 0;//回転度
	bool _isShotFlag = false;//ショットフラグ
	bool _isHit = false;//当たり判定
	bool _isVisible = false;//表示判定
	bool _isColVisible = false;//コリジョンの表示判定
	//Input用の変数
	IsKeyInput _isEnterInput{ false,false,KEY_INPUT_RETURN };//Enter.
	IsKeyInput _isUpInput{ false,false,KEY_INPUT_W };//Up.
	IsKeyInput _isDownInput{ false,false,KEY_INPUT_S };//Down.
	IsKeyInput _isLeftInput{ false,false,KEY_INPUT_A };//Left.
	IsKeyInput _isRightInput{ false,false,KEY_INPUT_D };//Right.
};

/*コンストラクタ*/
/// <summary>
/// Playerのコンストラクタ(インスタンス作成のみ)
/// </summary>
Player::Player(){}

/*Init*/
/// <summary>
/// メンバ変数の初期化
/// </summary>
/// <param name="drawHandle">drawハンドル</param>
/// <param name="x">X座標</param>
/// <param name="y">Y座標</param>
/// <param name="r">半径</param>
/// <param name="speed">スピード</param>
/// <param name="visible">表示フラグ</param>
/// /// <param name="colVisible">表示フラグ</param>
void Player::Init(int drawHandle, int x, int y, int r, int speed, bool visible, bool colVisible)
{
	_drawHandle = drawHandle;
	X = x;
	Y = y;
	_r = r;
	_speed = speed;
	_isVisible = visible;
	_isColVisible = colVisible;
}

/*Getter*/
/// <summary>
/// 弾丸を発射した際にTrue、それ以外はFalse
/// </summary>
/// <returns>bool</returns>
bool Player::getShotFlag()
{
	return _isShotFlag;
}
/// <summary>
/// 半径の取得
/// </summary>
/// <returns></returns>
int Player::getR()
{
	return _r;
}

/*メンバ関数*/
/// <summary>
/// 計算の更新
/// </summary>
void Player::Update()
{
	/*KeyUpdate*/
	_isEnterInput = InputKeyUpdate(_isEnterInput);//Enter.
	_isUpInput = InputKeyUpdate(_isUpInput);//Up.
	_isDownInput = InputKeyUpdate(_isDownInput);//Donw.
	_isLeftInput = InputKeyUpdate(_isLeftInput);//Left.
	_isRightInput = InputKeyUpdate(_isRightInput);//Right.

	/*移動処理*/
	//Up.
	if (_isUpInput.IsHold == true)
	{
		Y -= _speed;
	}
	//Down.
	if (_isDownInput.IsHold == true)
	{
		Y += _speed;
	}
	//Left.
	if (_isLeftInput.IsHold == true)
	{
		X -= _speed;
	}
	//Right.
	if (_isRightInput.IsHold == true)
	{
		X += _speed;
	}

	/*PlayerBulletの発射*/
	if (_isEnterInput.IsNow)//発射時
	{
		_isShotFlag = true;
	}
	else//非発射時
	{
		_isShotFlag = false;
	}

}
/// <summary>
/// 表示処理の更新
/// </summary>
void Player::Draw()
{
	//Playerの描画
	if (_isVisible)
	{
		DrawRotaGraph(X, Y, 1, 0, _drawHandle, 1);
	}
}
/// <summary>
/// Debug表示の更新
/// </summary>
void Player::DebugDraw()
{
	//コリジョンの描画
	if (_isColVisible)
	{
		DrawCircle(X, Y, _r, ColorCode::LIME, 0);
	}
}
