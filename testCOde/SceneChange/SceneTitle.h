#pragma once
#include "_baseScene.h"

class SceneTitle : public _baseScene
{
public:
	/*コンストラクタとデストラクタ*/
	SceneTitle() {};
	virtual ~SceneTitle() {};

	/*メンバ関数*/
	//純粋仮想関数のオーバーライド
	void Init() override;
	void Update() override;
	void Draw() override;
};

