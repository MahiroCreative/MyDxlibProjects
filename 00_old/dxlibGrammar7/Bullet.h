#pragma once
#include "MyDxlibCommon.h"
#include "DxLib.h"

class Bullet
{
public:
	/*publicメンバ変数*/
	int X=NULL;//X座標
	int Y=NULL;//Y座標
	/*コンストラクタ*/
	Bullet();
	/*Init*/
	void Init(int x,int y,int r,int speed,unsigned int color,bool visible);
	/*Getter*/
	bool getVisible();
	int getR();
	/*メンバ関数*/
	void Update();
	void Draw();
	void DebugDraw();
private:
	/*privateメンバ変数*/
	int _r=NULL;
	int _speed=NULL;
	unsigned int _color=NULL;
	bool _isVisible = false;
};

/*コンストラクタ*/
/// <summary>
/// Bulletのコンストラクタ
/// </summary>
Bullet::Bullet(){}

/*Init*/
/// <summary>
/// 初期化処理
/// </summary>
/// <param name="x">X座標</param>
/// <param name="y">Y座標</param>
/// <param name="r">半径</param>
/// <param name="speed">速度</param>
/// <param name="color">色</param>
/// <param name="visible">表示フラグ</param>
void Bullet::Init(int x, int y, int r, int speed, unsigned int color, bool visible)
{
	X = x;
	Y = y;
	_r = r;
	_speed = speed;
	_color = color;
	_isVisible = visible;
}

/*Getter*/
/// <summary>
/// 表示フラグの取得
/// </summary>
bool Bullet::getVisible()
{
	return _isVisible;
}
/// <summary>
/// 半径の取得
/// </summary>
/// <returns></returns>
int Bullet::getR()
{
	return _r;
}

/*メンバ関数*/
/// <summary>
/// 計算処理の更新
/// </summary>
void Bullet::Update()
{
	//画面内でのみ前進
	if (X > -80 && X < 1360)
	{
		X += _speed;
		//_isVisible = false;
	}
	else
	{
		_isVisible = false;
	}
}
/// <summary>
/// 表示処理の更新
/// </summary>
void Bullet::Draw()
{
	if (_isVisible)
	{
		DrawCircle(X, Y, _r, _color, 1);
	}

}
/// <summary>
/// Debug表示の更新
/// </summary>
void Bullet::DebugDraw()
{

}