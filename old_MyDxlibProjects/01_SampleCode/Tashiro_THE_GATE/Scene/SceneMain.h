#pragma once
#include "SceneBase.h"
#include <vector>
#include <list>
#include <memory>

struct Vec3;
class GateManager;
class StageManager;
class CameraManager;
class Player;
class UIMoveData;
enum class CameraKind;

class SceneMain final : public SceneBase
{
public:
	SceneMain(const wchar_t* const stageName);
	~SceneMain() {}

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
	void MainUpdate();
	void GameOverUpdate();

	void DrawNormal() const;
	void DrawBlend() const;
	void DrawGateBlend(int rt, CameraKind gate, CameraKind from) const;
	void DrawModelBlend(int rt, int tex1, int tex2, CameraKind camera, bool isPlayerDraw) const;

	void DrawCommonUI() const;
	void DrawExistUI() const;
	void DrawDeathUI() const;

private:
	using UpdateFunc_t = void(SceneMain::*)();
	UpdateFunc_t m_updateFunc;

	std::shared_ptr<GateManager> m_gateMgr;
	std::shared_ptr<StageManager> m_stageMgr;
	std::shared_ptr<CameraManager> m_cameraMgr;

	std::shared_ptr<Player> m_player;

	std::shared_ptr<UIMoveData> m_deathUIWindow;
	std::vector<std::shared_ptr<UIMoveData>> m_deathUIList;

	std::vector<int> m_rtTable;

	const wchar_t* const m_stageName;

	int m_depthRT;
	int m_depthRT2;
	int m_time;
	int m_current;
	int m_count;
};
