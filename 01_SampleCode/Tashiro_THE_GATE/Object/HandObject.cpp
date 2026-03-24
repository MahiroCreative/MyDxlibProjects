#include "HandObject.h"
#include "Collider/ColliderSphere.h"

HandObject::HandObject() :
	Object3DBase(Priority::MIDDLE, ObjectTag::HAND_OBJ)
{
}

HandObject::~HandObject()
{
}

void HandObject::Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity)
{
	Object3DBase::Init(pos, scale, rot, list, isGravity);
	m_startPos = pos;
}

void HandObject::Restart()
{
	m_rigid.SetPos(m_startPos);
}

void HandObject::OnHnad()
{
	m_rigid.SetGravity(false);
	m_throughTag.emplace_back(ObjectTag::PALYER);
}

void HandObject::EndHand()
{
	m_rigid.SetGravity(true);
	m_throughTag.pop_back();
}

void HandObject::SetPos(const Vec3& pos)
{
	m_rigid.SetPos(pos);
}

void HandObject::ZeroGravity()
{
	auto vel = m_rigid.GetVelocity();
	vel.y = 0;
	m_rigid.SetVelocity(vel);
}
