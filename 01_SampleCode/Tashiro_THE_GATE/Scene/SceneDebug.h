#pragma once
#ifdef _DEBUG
#include "SceneBase.h"
class SceneDebug final : public SceneBase
{
public:
	SceneDebug();
	~SceneDebug() {}

	// シーンに入るときの初期化処理(非同期)
	void AsyncInit() override;
	// シーンに入るときの初期化処理(同期)
	void Init() override;
	// シーンを抜けるときの処理
	void End() override;
	// 更新処理(同期)
	void Update(bool isFade) override;
	// 描画処理
	void Draw() const override;

private:
	int m_next;
};
#endif
