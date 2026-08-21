#include "OverScene.h"

void OverScene::Init()
{
	//変数初期化
	_nextScene = GameSetting::SceneState::OverGame;	
	//初期化
	_overText.Init("-GameOver-", Vector2{ GameSetting::WINDOW_CENTER_X -320,GameSetting::WINDOW_CENTER_Y -100 });
	_overText.Transform.Scale = Vector2{ 6,6 };
}

int OverScene::Update()
{
	//Key入力の更新
	InputKey::Update();

	//何かしらのKeyが押されたらタイトルに戻る
	if (InputKey::isAnyKey()) { _nextScene = GameSetting::SceneState::Title; }


	//int型に変換して次のシーンを返す
	return static_cast<int>(_nextScene);
}

void OverScene::Draw()
{
	DrawString(0,0,"OverScene",Color::WhiteColor);

	//シーンロゴ表示
	_overText.Draw();
}
