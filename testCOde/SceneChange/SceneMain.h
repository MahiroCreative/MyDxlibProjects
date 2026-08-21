#pragma once
#include "_baseScene.h"
class SceneMain :
    public _baseScene
{
public:
	/*コンストラクタとデストラクタ*/
	SceneMain() {};
	virtual ~SceneMain() {};

	//純粋仮想関数
	void Init() override;
	void Update() override;
	void Draw() override;

};

