#include "Game.h"
#include "DxLib.h"

Game& Game::GetInstance()
{
	static Game instance;
	return instance;
}

bool Game::Initialize()
{
	// TRUEでウィンドウモード起動(FALSEにするとフルスクリーンになる)。
	ChangeWindowMode(TRUE);
	// ウィンドウの解像度と色深度(32bitカラー)を設定する。
	// DxLib_Init()より前に呼ぶ必要がある。
	// サイズはGridが持つマス目の情報から自動計算する(GridとGameでサイズの数値を二重管理しない)。
	SetGraphMode(m_grid.GetScreenWidth(), m_grid.GetScreenHeight(), 32);

	// DxLibの初期化。失敗したら(-1が返る)呼び出し元にfalseを伝える。
	if (DxLib_Init() == -1) return false;

	// 描画先を裏画面(バックバッファ)に設定する。
	// 直接表画面に描くとちらつきが出るため、裏で描いてから ScreenFlip() で一気に表示を切り替える
	// 「ダブルバッファリング」という定石の手法。
	SetDrawScreen(DX_SCREEN_BACK);

	return true;
}

void Game::Run()
{
	// メインループ。
	// ProcessMessage()はOSからのウィンドウメッセージ処理(×ボタンなど)で、
	// 通常は0が返り、ウィンドウが閉じられようとすると0以外が返ってループを抜ける。
	// ClearDrawScreen()は裏画面を1色でクリアする関数で、これも通常0が返る。
	while (ProcessMessage() == 0 && ClearDrawScreen() == 0)
	{
		Update();
		Draw();

		// 裏画面に描いた内容を表画面へ反映する(画面の更新はここで初めて起こる)。
		ScreenFlip();

		// ESCキーが押されたらループを抜けてゲームを終了する。
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
	}

	Shutdown();
}

void Game::Update()
{
	// 現時点では状態を更新する処理がないので空。
	// 今後、採掘機の生産処理やベルトの搬送処理などをここに足していく。
}

void Game::Draw()
{
	m_grid.DrawGridLines(m_camera);
	m_grid.DrawHoverHighlight(m_camera);
	m_grid.DrawTilePositionText(m_camera);
}

void Game::Shutdown()
{
	// DxLibの後始末。DxLib_Init()と対で必ず呼ぶ。
	DxLib_End();
}
