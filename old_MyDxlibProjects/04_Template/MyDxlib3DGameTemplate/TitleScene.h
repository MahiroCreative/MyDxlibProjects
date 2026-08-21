#pragma once
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"
class TitleScene :
    public _baseGameScene
{
private:
	/*メンバ変数*/
	// //タイマー用
	int _arrowTimer = 0;
	//矢印のタイマー切り替え用
	bool _arrowTimerSwitch = false;
	//setScene.
	GameSetting::SceneState _setScene = GameSetting::SceneState::Title;
	//nextScene.
	GameSetting::SceneState _nextScene = GameSetting::SceneState::Game;

public:
	//コンストラクタ
	TitleScene() = default;
	//デストラクタ
	~TitleScene() = default;

    /*メンバ関数*/
	void Init() override;
	void Update() override;
	void Draw() override;
};

