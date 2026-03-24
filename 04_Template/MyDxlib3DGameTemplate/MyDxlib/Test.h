#pragma once
#include "DxLib.h"
#include "MyDxlib.h"


struct Transform
{
	Vector3 Position;
	Vector3 Rotation;
	Vector3 Scale;
};

/// <summary>
/// オブジェクトの基底クラス
/// </summary>
class _baseGameObejct
{
private:
	//デストラクタ
	virtual ~_baseGameObejct() = 0;
	//メンバ変数

};