#pragma once
//自作ベクトル2Dクラス
#include "../MyDxlibMath/MyDxlibMath.h"

struct Rigidbody2D
{
	//速度
	Vector2 Velocity;
	//加速度
	Vector2 Acceleration;
	//質量
	float Mass=0;
};