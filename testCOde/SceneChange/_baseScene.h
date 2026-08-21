#pragma once
class _baseScene
{
public:
	//メンバ関数
	virtual ~_baseScene() = default;//defaultじゃないとエラーが出る
	//純粋仮想関数
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};