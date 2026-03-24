#pragma once
//STL.
#include <memory>
//origin.
#include "MyDxlib/MyDxlib.h"
#include "GameSetting.h"

//タイトルシーンクラス
class TitleScene : public _baseGameScene
{
public:
	/*コンストラクタデストラクタ*/
	//コンストラクタ
	TitleScene()
		: _arrowTimer(),
		_arrowTimerSwitch(),
		_setScene(),
		_nextScene(),
		_arrow(),
		_shootingGameText(),
		_platformGameText(),
		_endGameText()
	{
		Init();
	}
	//デストラクタ
	~TitleScene() = default;

	/*定型メンバ関数*/
	//初期化
	void Init() override;
	//更新
	int Update() override;
	//描画
	void Draw() override;

	/*メンバ関数*/
	//矢印の更新
	void ArrowUpdate();
	//stateの更新
	void StateUpdate();
	//シーンの決定
	void SceneDecision();

private:
	/*処理変数*/
	//タイマー用
	int _arrowTimer;
	//矢印のタイマー切り替え用
	bool _arrowTimerSwitch;
	//setScene.
	GameSetting::SceneState _setScene;
	//nextScene.
	GameSetting::SceneState _nextScene;

	/*ゲームオブジェクト*/
	//Arrow.シーン選択用の矢印。
	std::unique_ptr<SimpleText> _arrow;
	//ShootingGame.シーン選択用のテキスト。
	std::unique_ptr<SimpleText> _shootingGameText;
	//PlatformGame.シーン選択用のテキスト。
	std::unique_ptr<SimpleText> _platformGameText;
	//EndGame.シーン選択用のテキスト。
	std::unique_ptr<SimpleText> _endGameText;
};

