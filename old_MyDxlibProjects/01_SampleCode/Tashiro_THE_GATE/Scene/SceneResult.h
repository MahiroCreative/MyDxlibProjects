#pragma once
#include "SceneBase.h"
#include <array>

class SceneResult : public SceneBase
{
private:
	enum class DecideTime
	{
		DEC,
		SEC,
		MIN,
		MAX
	};

public:
	SceneResult(const wchar_t* const stageName, const int clearTime);
	~SceneResult() {}

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
	void TimeUpdate();
	void StampUpdate();
	void ScalingUpdate();
	void SelectUpdate();

	void DrawTime() const;
	void DrawTime(int x, int y, DecideTime kind, int time) const;
	void DrawRank() const;
	void DrawRankingUpdate() const;

private:
	const wchar_t* const m_stageName;
	const int m_clearTime;

	using ProcessUpdate_t = void(SceneResult::*)();
	ProcessUpdate_t m_processFunc;

	std::array<int, static_cast<size_t>(DecideTime::MAX)> m_moveCount[3];
	int m_count;
	int m_indexDecide;

	float m_scale;

	std::array<bool, static_cast<size_t>(DecideTime::MAX)> m_isDecide[3];
	std::array<bool, static_cast<size_t>(DecideTime::MAX)> m_isMove[3];
	bool m_isAllDecide;
	bool m_isAllMoved;
	bool m_isRankingUpdate;
};

