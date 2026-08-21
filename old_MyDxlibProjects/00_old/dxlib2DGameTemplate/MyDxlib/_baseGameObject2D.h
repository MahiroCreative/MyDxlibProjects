#pragma once

//ゲームオブジェクト基底クラス
class _baseGameObject2D
{
public:
	//メンバ関数
	virtual ~_baseGameObject2D() = default;//defaultじゃないとエラーが出る
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

};