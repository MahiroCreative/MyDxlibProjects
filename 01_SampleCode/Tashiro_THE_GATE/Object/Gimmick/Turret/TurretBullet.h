#pragma once
#include "Object/Object3DBase.h"

namespace MyEngine
{
	struct CollideHitInfo;
}
class Player;

class TurretBullet : public Object3DBase
{
public:
	TurretBullet();
	~TurretBullet();

	void Init(const Vec3& pos, const Vec3& dir, Player* player);

	bool IsExist() const { return m_isExist; }

	virtual void OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	Player* m_player;

	bool m_isExist;
};

