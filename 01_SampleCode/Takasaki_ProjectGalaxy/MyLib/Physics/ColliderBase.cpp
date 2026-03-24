#include "ColliderBase.h"

MyEngine::ColliderBase::ColliderBase(Kind kind,ColideTag tag) :
	isTrigger(false),
	m_kind(kind),
	m_tag(tag),
	m_posShift(Vec3::Zero())
{
	m_isPreHit.clear();
	m_isHit.clear();
}

void MyEngine::ColliderBase::UpdateHit(const ColliderBase* collider, bool isHit)
{
	m_isPreHit[collider] = m_isHit[collider];
	m_isHit[collider] = isHit;
}
