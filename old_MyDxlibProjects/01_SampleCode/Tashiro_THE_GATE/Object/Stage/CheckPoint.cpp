#include "CheckPoint.h"
#include "SaveDataManager.h"
#include "StageManager.h"
#include "Quaternion.h"
#include "Collider/ColliderCapsule.h"
#include "Object/Gimmick/GimmickLinkObject.h"

CheckPoint::CheckPoint(StageManager& mgr, const int no) :
	Collidable(Priority::STATIC, ObjectTag::CHECK_POINT),
    m_mgr(mgr),
    m_no(no),
    m_link(nullptr)
{
}

CheckPoint::~CheckPoint()
{
}

void CheckPoint::Init(const Vec3& pos, const Quaternion& rot, float size, float radius)
{
    OnEntryPhysics();
    m_rigid.SetPos(pos);
    m_rigid.SetGravity(false);

    const auto& sphere = std::dynamic_pointer_cast<MyEngine::ColliderCapsule>(CreateCollider(MyEngine::ColKind::CAPSULE));
    sphere->center = Vec3();
    sphere->isTrigger = true;
    sphere->dir = rot * Vec3::Up();
    sphere->size = size;
    sphere->radius = radius;
}


void CheckPoint::OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
    MyEngine::Collidable::OnTriggerEnter(colider, selfIndex, sendIndex, hitInfo);

    auto tag = colider->GetTag();
    if (tag == ObjectTag::PALYER)
    {
        m_mgr.UpdateCheckPoint(m_no);
        if (m_link)
        {
            m_link->OnGimmick();
        }
    }
}
