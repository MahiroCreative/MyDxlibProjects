#pragma once
#include "DxLib.h"
#include "Vector2.h"

struct CircleCollision2D
{
	/*メンバ変数*/
	//中心座標
	Vector2 Center;
	//半径
	float Radius;
	//Debug用Drawの色
	unsigned int Color;

	/*メンバ関数*/
	//コリジョンの描画
	void Draw()
	{
		DrawCircle(static_cast<int>(Center.X), static_cast<int>(Center.Y), static_cast<int>(Radius), Color, FALSE);
	}

	//コリジョンの判定
	bool IsCollision(CircleCollision2D col)
	{
		//2点間の距離
		float distance = (Center - col.Center).Length();
		//半径の和
		float sumRadius = Radius + col.Radius;
		//判定
		if (distance < sumRadius)
		{
			return true;
		}
		return false;
	}

};