#pragma once
#include "Scene.h"
class PauseScene : public Scene
{
public:
	PauseScene(SceneManager& mgr);

	virtual void Load();
	void Update();
	void Draw();

private:
	int m_frameHandle;
	int m_btnFrame;
	int m_fadeSpeed;
	int m_frame = 0;
	int m_select = 0;
	int m_soundVol;
	int m_fontHandle;

	int m_tutoHandle = 0;

	bool m_tutoFlag = false;
	// 更新メンバ関数ポインタ
	using UpdateFunc_t = void(PauseScene::*)();
	UpdateFunc_t  m_updateFunc;
	// 描画メンバ関数ポインタ
	using DrawFunc_t = void (PauseScene::*)();
	DrawFunc_t m_drawFunc;

	// 更新関数
	void AppearUpdate();	// 登場状態
	void NormalUpdate();		// 通常状態
	void DisappearUpdate();	// 退場状態

	// 描画関数
	void ExpandDraw();	// 拡張縮張描画
	void NormalDraw();	// 非フェード描画
};

