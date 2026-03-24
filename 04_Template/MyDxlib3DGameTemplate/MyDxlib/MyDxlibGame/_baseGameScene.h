#pragma once

//ゲームシーン基底クラス
class _baseGameScene
{
public:
	//メンバ関数
	virtual ~_baseGameScene() = default;//defaultじゃないとエラーが出る
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};