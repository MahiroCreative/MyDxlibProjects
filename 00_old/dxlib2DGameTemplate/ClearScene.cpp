#include "ClearScene.h"

void ClearScene::Init()
{
	//変数初期化
	_nextScene = GameSetting::SceneState::ClearGame;
	//初期化
	_clearText.Init("-GameClear-", Vector2{ GameSetting::WINDOW_CENTER_X - 320,GameSetting::WINDOW_CENTER_Y - 100 });
	_clearText.Transform.Scale = Vector2{ 6,6 };

}

int ClearScene::Update()
{
	//Key入力の更新
	InputKey::Update();

	//何かしらのKeyが押されたらタイトルに戻る
	if (InputKey::isAnyKey()) { _nextScene = GameSetting::SceneState::Title; }

	//int型に変換して次のシーンを返す
	return static_cast<int>(_nextScene);
}

void ClearScene::Draw()
{
	DrawString(0, 0, "ClearScene", Color::WhiteColor);

	//シーンロゴ表示
	_clearText.Draw();
}