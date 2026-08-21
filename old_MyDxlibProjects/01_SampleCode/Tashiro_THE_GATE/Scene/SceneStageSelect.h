#pragma once
#include "SceneBase.h"

class SceneStageSelect : public SceneBase
{
public:
	SceneStageSelect();
	~SceneStageSelect() {}

	// 初期化処理(非同期)
	void AsyncInit() override;
	// 初期化処理(同期)
	void Init() override;
	// 終了処理
	void End() override;
	// 更新処理(同期)
	void Update(bool isFade) override;
	// 描画処理
	void Draw() const override;

private:
	void SelectUpdate();

private:
	int m_selectCurrent;

	int m_count;
};

