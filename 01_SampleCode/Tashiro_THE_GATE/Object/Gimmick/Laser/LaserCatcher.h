#pragma once
#include "Object/Gimmick/GimmickSendObject.h"

namespace MyEngine
{
	struct CollideHitInfo;
}
class GimmickLinkObject;
class FileBase;

class LaserCatcher : public GimmickSendObject
{
public:
	LaserCatcher();
	virtual ~LaserCatcher();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;

	virtual void OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	std::shared_ptr<FileBase> m_catchSe;
};
