#include "Collidable.h"
#include <cassert>
#include "Vec3.h"
#include "DebugDraw.h"
#include "Physics.h"
#include "Collider/Colliders.h"

using namespace MyEngine;

Collidable::Collidable(Priority priority, ObjectTag tag) :
	m_priority(priority),
	m_tag(tag),
	m_isEntry(false)
{
	m_groundTag.emplace_back(ObjectTag::FLOOR);
	m_groundTag.emplace_back(ObjectTag::FLOOR_MOVE);
	m_groundTag.emplace_back(ObjectTag::NO_GATE_FLOOR);
	m_groundTag.emplace_back(ObjectTag::ROOF);
	m_groundTag.emplace_back(ObjectTag::NO_GATE_ROOF);
	m_groundTag.emplace_back(ObjectTag::STAIRS);
	m_groundTag.emplace_back(ObjectTag::CATWALK);
}

Collidable::~Collidable()
{
}

void Collidable::End()
{
	if (m_isEntry)
	{
		auto& physics = Physics::GetInstance();
		physics.Exit(shared_from_this());
		m_isEntry = false;
	}
}

ColliderBase* Collidable::GetColliderData(int index) const
{
	return m_collider.at(index).get();
}

void Collidable::OnCollideEnter(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Collide", "当たった", colider, selfIndex, sendIndex);
#endif
}

void Collidable::OnCollideStay(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Collide", "当たっている", colider, selfIndex, sendIndex);
#endif
	const auto& tag = colider->GetTag();
	const auto& isFind = std::find(m_groundTag.begin(), m_groundTag.end(), tag) != m_groundTag.end();
	if (isFind)
	{
		auto vel = m_rigid.GetVelocity();
		vel.y = 0;
		m_rigid.SetVelocity(vel);
	}
}

void Collidable::OnCollideExit(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Collide", "離れた", colider, selfIndex, sendIndex);
#endif
}

void Collidable::OnTriggerEnter(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Trigger", "当たった", colider, selfIndex, sendIndex);
#endif
}

void Collidable::OnTriggerStay(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Trigger", "当たっている", colider, selfIndex, sendIndex);
#endif
}

void Collidable::OnTriggerExit(Collidable* colider, int selfIndex, int sendIndex, const CollideHitInfo& hitInfo)
{
#ifdef _DEBUG
	SendCollideInfo("Trigger", "離れた", colider, selfIndex, sendIndex);
#endif
}

const Vec3& Collidable::GetPos() const
{
	return m_rigid.GetPos();
}

const Vec3& MyEngine::Collidable::GetVelocity() const
{
	return m_rigid.GetVelocity();
}

void Collidable::AddForce(const Vec3& dir, float power)
{
	m_rigid.AddForce(dir, power);
}

/// <summary>
/// Physicsに追加する
/// </summary>
void Collidable::OnEntryPhysics()
{
	if (m_isEntry) return;
	auto& physics = MyEngine::Physics::GetInstance();
	physics.Entry(shared_from_this());
	m_isEntry = true;
}

/// <summary>
/// Physicsから削除する
/// </summary>
void Collidable::OnExistPhysics()
{
	if (!m_isEntry) return;
	auto& physics = MyEngine::Physics::GetInstance();
	physics.Exit(shared_from_this());
	m_isEntry = false;
}

std::shared_ptr<ColliderBase> Collidable::CreateCollider(const ColKind& kind)
{
	std::shared_ptr<ColliderBase> col;
	if (kind == ColKind::SPHERE)
	{
		col = std::make_shared<ColliderSphere>();
	}
	else if (kind == ColKind::CAPSULE)
	{
		col = std::make_shared<ColliderCapsule>();
	}
	else if (kind == ColKind::TERRAIN)
	{
		col = std::make_shared<ColliderTerrain>();
	}
	else if (kind == ColKind::BOX)
	{
		col = std::make_shared<ColliderBox>();
	}

	if (!col)
	{
		assert(false);
		col = std::make_shared<ColliderSphere>();
	}

	m_collider.emplace_back(col);

	return col;
}

#ifdef _DEBUG
#include "ObjectTag.h"
void Collidable::SendCollideInfo(const char* start, const char* const end, Collidable* collider, int selfIndex, int sendIndex)
{
	printf("%s:%sが%sと%s\n", start, Tags::TAG_NAME.at(m_tag), Tags::TAG_NAME.at(collider->GetTag()), end);
	printf("selfIndex: %d/ sendIndex: %d\n", selfIndex, sendIndex);
}
#endif