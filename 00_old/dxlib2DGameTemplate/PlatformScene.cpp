#include "PlatformScene.h"

namespace
{
	//リターンキーの定義
	const int _kReturnKey = KEY_INPUT_B;
}

/*メンバ関数*/
void PlatformScene::Init()
{
	//変数初期化
	_nextScene = GameSetting::SceneState::PlatformGame;

	//ゲームオブジェクトの生成
	_player = std::make_shared<PlatformPlayer>();

	//初期化
	_player->Init();
}

int PlatformScene::Update()
{
	//Key入力の更新
	InputKey::Update();

	//Playerの更新
	_player->Update();

	//Enterキーが押されたらタイトルシーンに遷移
	if (InputKey::isDownKey(_kReturnKey))
	{
		_nextScene = GameSetting::SceneState::Title;
	}

	//int型に変換して返す
	return static_cast<int>(_nextScene);
}

void PlatformScene::Draw()
{
	//現在のシーン名を表示
	DrawString(0, 0, "PlatformScene: AD(左右),Space(ジャンプ),B(タイトル)", GetColor(255, 255, 255));
	//Playerの描画
	_player->Draw();

	//地面の描画
	DrawLine(0, 640, 1280, 640, GetColor(255, 255, 255));
}
