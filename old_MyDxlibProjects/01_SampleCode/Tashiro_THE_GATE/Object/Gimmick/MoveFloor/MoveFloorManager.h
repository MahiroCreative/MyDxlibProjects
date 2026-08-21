#pragma once
#include "Object/Gimmick/GimmickLinkObject.h"
#include <vector>
#include <list>
#include <memory>

class MoveFloor;

class MoveFloorManager : public GimmickLinkObject
{
public:
	MoveFloorManager();
	~MoveFloorManager();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
	void Restart() override;
	void Draw() const override;

	void SetLoop(bool isLoop);
	void SetCheckPtTable(const std::vector<Vec3>& table) { m_checkPtTable = table; }
	int GetCheckPtNum() const { return static_cast<int>(m_checkPtTable.size()); }
	const Vec3& GetCheckPtPos(int no) const { return m_checkPtTable.at(no); }
	Vec3 GetMoveDir(const Vec3& pos, int no) const;

	bool CheckMovedCheckPt(const Vec3& pos, int no);

private:
	void GimmickOnUpdate() override;
	void GimmickOffUpdate() override;

	void CreateUpdate();
	void FloorUpdate();

	void ProcessGimmickOn() override;
	void ProcessGimmickOff() override;

private:
	std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> m_colList;
	std::list<std::shared_ptr<MoveFloor>> m_floorList;

	std::vector<Vec3> m_checkPtTable;

	int m_createFrame;

	bool m_isLoop;
};
