#pragma once

#include "Camera.h"
#include "Grid.h"

// アプリ全体の土台となるクラス。
// DxLibの初期化/終了とメインループ(Update→Draw→ScreenFlip)を持ち、
// main.cppからはこのクラスを1つ使うだけで済むようにする。
//
// ゲーム全体で存在するのは1つだけでよい(2つ目を作る意味がない)ため、シングルトンにしている。
// GetInstance()の中の static Game instance は、初めて呼ばれたときに1回だけ生成され、
// 以降は同じインスタンスが使い回される(C++11以降ではこの初期化はスレッドセーフと規格で保証されている)。
class Game
{
public:
	// 唯一のインスタンスを取得する。
	static Game& GetInstance();

	// コピー・ムーブを禁止する。
	// シングルトンは「常に1つしか存在しない」ことが前提なので、
	// コピーできてしまうと2つ目のインスタンスが作れてしまい前提が崩れる。
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(Game&&) = delete;

	// DxLibの初期化を行う。失敗したらfalseを返す。
	bool Initialize();

	// メインループを実行する。ウィンドウが閉じられるかESCが押されると終了する。
	void Run();

private:
	// コンストラクタをprivateにすることで、GetInstance()経由以外での生成を禁止する。
	Game() = default;

	void Update();
	void Draw();
	void Shutdown();

	Camera m_camera;
	Grid m_grid;
};
