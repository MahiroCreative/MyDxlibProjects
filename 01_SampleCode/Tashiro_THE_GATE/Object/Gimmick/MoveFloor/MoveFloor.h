#pragma once
#include "Object/Object3DBase.h"

class MoveFloorManager;

class MoveFloor : public Object3DBase
{
public:
	MoveFloor(MoveFloorManager& mgr);
	~MoveFloor();

	void InitDir(const Vec3& dir);
	void Update() override;

	void SetFunc(bool isLoop);

	bool IsEnd() const { return m_isEnd; }

	void StopMove();
	void ReStartMove();

private:
	void LoopModeUpdate();
	void NoLoopModeUpdate();

private:
	using UpdateFunc_t = void(MoveFloor::*)();
	UpdateFunc_t m_updateFunc;

	MoveFloorManager& m_mgr;

	Vec3 m_saveVel;

	int m_nowCheckPt;
	int m_nextCheckPt;
	int m_count;

	bool m_isEnd;
	bool m_isStop;
	bool m_isArrival;
};

