#include "Collidable.h"
#include "ColliderSphere.h"
#include<cassert>

using namespace MyEngine;

Collidable::Collidable(Priority priority, ObjectTag tag) :
	m_priority(priority),
	m_tag(tag),
	m_isAntiGravity(false)
{
	m_rigid = std::make_shared<Rigidbody>();
}

MyEngine::Collidable::Collidable(std::shared_ptr<Collidable> col) :
	m_rigid(col->m_rigid),
	m_colliders(col->m_colliders),
	m_upVec(col->m_upVec),
	throughTags(col->throughTags),
	m_tag(col->m_tag),
	m_priority(col->m_priority)
{
	m_rigid = std::make_shared<Rigidbody>();
}

Collidable::~Collidable()
{
}

std::shared_ptr<ColliderBase> MyEngine::Collidable::AddCollider(const ColliderBase::Kind& kind)
{
	std::shared_ptr<ColliderBase> collider;

	if (kind == ColliderBase::Kind::Sphere)
	{
		collider = std::make_shared<ColliderSphere>();
	}
	else if (kind == ColliderBase::Kind::Capsule)
	{

	}

	if (collider)
	{
		m_colliders.emplace_back(collider);
	}

	return collider;
}
/// <summary>
/// 当たり判定を無視（スルー）するタグの追加
/// </summary>
void Collidable::AddThroughTag(ObjectTag tag)
{
	bool found = (std::find(throughTags.begin(), throughTags.end(), tag) != throughTags.end());
	if (found)
	{
		assert(0 && L"指定タグは既に追加されています");
	}
	else
	{
		throughTags.emplace_back(tag);
	}
}

/// <summary>
/// 当たり判定を無視（スルー）するタグの削除
/// </summary>
void Collidable::RemoveThroughTag(ObjectTag tag)
{
	bool found = (std::find(throughTags.begin(), throughTags.end(), tag) != throughTags.end());
	if (!found)
	{
		assert(0 && L"指定タグは存在しません");
	}
	else
	{
		throughTags.remove(tag);
	}
}

// 当たり判定を無視（スルー）する対象かどうか
bool Collidable::IsThroughTarget(const std::shared_ptr<Collidable> target) const
{
	bool found = (std::find(throughTags.begin(), throughTags.end(), target->GetTag()) != throughTags.end());
	return found;
}