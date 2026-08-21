#pragma once
#include "Collidable.h"

struct Quaternion;
class StageManager;
class GimmickLinkObject;

class CheckPoint : public MyEngine::Collidable
{
public:
	CheckPoint(StageManager& mgr, const int no);
	~CheckPoint();

	void Init(const Vec3& pos, const Quaternion& rot, float size, float radius);
	void SetLinkObj(GimmickLinkObject* link) { m_link = link; }

	virtual void OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	StageManager& m_mgr;
	const int m_no;

	GimmickLinkObject* m_link;
};

