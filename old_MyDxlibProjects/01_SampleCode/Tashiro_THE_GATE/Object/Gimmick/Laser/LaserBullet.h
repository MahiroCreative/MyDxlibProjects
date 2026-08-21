#pragma once
#include "Object/Object3DBase.h"

namespace MyEngine
{
	struct CollideHitInfo;
}
class GateManager;
class LaserLaunchPad;

class LaserBullet : public Object3DBase
{
public:
	LaserBullet(LaserLaunchPad* launcPad, const GateManager* gateMgr);
	~LaserBullet();

	void Init(const Vec3& pos, const Vec3& dir, int existTime);
	void Update() override;

	virtual void OnCollideEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;
	virtual void OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;
	virtual void OnTriggerStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	const GateManager* m_gateMgr;
	LaserLaunchPad* m_launchPad;

	int m_time;

	bool m_isAddThroughTag;
	bool m_isWarp;
};

