#include "TitleScene.h"

namespace
{
	//矢印の移動量
	constexpr float _kMoveArrow = 40.0f;
	//矢印の表示切替時間
	constexpr int _kArrowShowTime = 60;
	//矢印の非表示切替時間
	constexpr int _kArrowDeleteTime = 30;
	//矢印の初期位置
	const Vector2 _kArrowFirstPos = Vector2(GameSetting::WINDOW_CENTER_X - 120.0f, 280.0f);
	//1番目のテキストの表示位置
	const Vector2 _kFirstTextPos = Vector2(GameSetting::WINDOW_CENTER_X - 80.0f, 280.0f);
	//2番目のテキストの表示位置
	const Vector2 _kSecondTextPos = Vector2(GameSetting::WINDOW_CENTER_X - 80.0f, 320.0f);
	//3番目のテキストの表示位置
	const Vector2 _kThirdTextPos = Vector2(GameSetting::WINDOW_CENTER_X - 80.0f, 360.0f);

}

/*メンバ関数*/
//初期化
void TitleScene::Init()
{
	/*処理変数の初期化*/
	_arrowTimer = 0;
	_arrowTimerSwitch = false;
	_setScene = GameSetting::SceneState::ShootingGame;
	_nextScene = GameSetting::SceneState::Title;

	/*ポインタの初期化*/
	//矢印
	_arrow = std::make_unique<SimpleText>();
	//シューティングゲーム
	_shootingGameText = std::make_unique<SimpleText>();
	//プラットフォームゲーム
	_platformGameText = std::make_unique<SimpleText>();
	//ゲーム終了
	_endGameText = std::make_unique<SimpleText>();

	/*オブジェクトの初期化*/
	//矢印
	_arrow->Init("->");
	_arrow->SetColor(Color::RedColor);
	//シューティングゲーム
	_shootingGameText->Init("ShootingGame");
	//プラットフォームゲーム
	_platformGameText->Init("PlatformGame");
	//ゲーム終了
	_endGameText->Init("EndGame");

	/*オブジェクトの位置設定*/
	//矢印
	_arrow->Transform.Position = _kArrowFirstPos;
	//シューティングゲーム
	_shootingGameText->Transform.Position = _kFirstTextPos;
	//プラットフォームゲーム
	_platformGameText->Transform.Position = _kSecondTextPos;
	//ゲーム終了
	_endGameText->Transform.Position = _kThirdTextPos;
}

int TitleScene::Update()
{
	//Key入力の更新
	InputKey::Update();
	//矢印の更新
	ArrowUpdate();
	//stateの更新
	StateUpdate();
	//シーンの決定
	SceneDecision();

	//int型に変換して返す
	return static_cast<int>(_nextScene);
}

void TitleScene::Draw()
{
	/*現在のシーンの表示*/
	//シーン名
	DrawString(0, 0, "TitleScene", Color::WhiteColor);

	/*オブジェクトの描画*/
	//矢印
	_arrow->Draw();
	//シューティングゲーム
	_shootingGameText->Draw();
	//プラットフォームゲーム
	_platformGameText->Draw();
	//ゲーム終了
	_endGameText->Draw();
}

void TitleScene::ArrowUpdate()
{
	//矢印の移動
	if (InputKey::isDownKey(KEY_INPUT_DOWN))
	{
		_arrow->Transform.Position.Y += _kMoveArrow;
	}
	else if (InputKey::isDownKey(KEY_INPUT_UP))
	{
		_arrow->Transform.Position.Y -= _kMoveArrow;
	}

	//矢印の移動範囲
	if (_arrow->Transform.Position.Y < _shootingGameText->Transform.Position.Y)
	{
		_arrow->Transform.Position.Y = _endGameText->Transform.Position.Y;
	}
	else if (_arrow->Transform.Position.Y > _endGameText->Transform.Position.Y)
	{
		_arrow->Transform.Position.Y = _shootingGameText->Transform.Position.Y;
	}

	//タイマの更新
	_arrowTimer++;

	//矢印の表示切替
	if (_arrow->Visible)//矢印が表示されている
	{
		//60frameで消す
		if (_arrowTimer > _kArrowShowTime)
		{
			_arrow->Visible = false;
			_arrowTimerSwitch = true;
		}

	}
	else//矢印が表示されていない
	{
		//30frameで表示
		if (_arrowTimer > _kArrowDeleteTime)
		{
			_arrow->Visible = true;
			_arrowTimerSwitch = true;
		}
	}

	//タイマの初期化
	if (_arrowTimerSwitch)
	{
		_arrowTimer = 0;
		_arrowTimerSwitch = false;
	}
}

void TitleScene::StateUpdate()
{
	//矢印の位置によってstateを変更
	if (_arrow->Transform.Position.Y == _shootingGameText->Transform.Position.Y)
	{
		_setScene = GameSetting::SceneState::ShootingGame;
	}
	else if (_arrow->Transform.Position.Y == _platformGameText->Transform.Position.Y)
	{
		_setScene = GameSetting::SceneState::PlatformGame;
	}
	else if (_arrow->Transform.Position.Y == _endGameText->Transform.Position.Y)
	{
		_setScene = GameSetting::SceneState::EndGame;
	}
}

void TitleScene::SceneDecision()
{
	//決定ボタンが押されたら現在セットされているシーンに決定する
	if (InputKey::isDownKey(KEY_INPUT_RETURN))
	{
		_nextScene = _setScene;
	}
}
