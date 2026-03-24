#include "LaserCatcher.h"
#include "File.h"
#include "SoundManager.h"
#include "Collider/ColliderSphere.h"
#include "Object/Gimmick/GimmickLinkObject.h"

namespace
{
	const wchar_t* const FILE_CATCH = L"S_EnergyCatch";
}

LaserCatcher::LaserCatcher() :
	GimmickSendObject(Priority::STATIC, ObjectTag::LASER_CATCHER)
{
}

LaserCatcher::~LaserCatcher()
{
}

void LaserCatcher::Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity)
{
	Object3DBase::Init(pos, scale, rot, list, isGravity);

	m_catchSe = FileManager::GetInstance().Load(FILE_CATCH);
}

void LaserCatcher::OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerEnter(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::LASER_BULLET)
	{
		SoundManager::GetInstance().PlaySe3D(m_catchSe->GetHandle(), shared_from_this());
		m_linkObj->OnGimmick();
	}
}
