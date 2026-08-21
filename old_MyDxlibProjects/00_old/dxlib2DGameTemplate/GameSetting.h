#pragma once

namespace GameSetting
{
	// ウィンドウタイトル
	constexpr const char* WINDOW_TITLE = "MyDxlib2DGame";
	// ウィンドウサイズ
	constexpr int WINDOW_WIDTH = 1280;
	constexpr int WINDOW_HEIGHT = 720;
	//ウィンドウ中央座標
	constexpr int WINDOW_CENTER_X = WINDOW_WIDTH / 2;
	constexpr int WINDOW_CENTER_Y = WINDOW_HEIGHT / 2;

	//SceneState.
	enum class SceneState
	{
		Title,
		ShootingGame,
		PlatformGame,
		EndGame,
		OverGame,
		ClearGame
	};
}