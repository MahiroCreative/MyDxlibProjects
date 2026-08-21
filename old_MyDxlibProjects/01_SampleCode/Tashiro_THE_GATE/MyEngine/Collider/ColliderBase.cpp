#include "ColliderBase.h"
#include <cassert>
#include "CollideHelp.h"

using namespace MyEngine;

ColliderBase::ColliderBase(ColKind kind) :
	isTrigger(false),
	center(Vec3()),
	m_kind(kind)
{
	m_collideFunc[ColKind::SPHERE] = &ColliderBase::IsCollideSphere;
	m_collideFunc[ColKind::CAPSULE] = &ColliderBase::IsCollideCapsule;
	m_collideFunc[ColKind::TERRAIN] = &ColliderBase::IsCollideTerrain;
	m_collideFunc[ColKind::BOX] = &ColliderBase::IsCollideBox;
	m_collideFunc[ColKind::LINE] = &ColliderBase::IsCollideLine;
	m_collideFunc[ColKind::WALL] = &ColliderBase::IsCollideWall;

	m_fixFunc[ColKind::SPHERE] = &ColliderBase::FixToSphere;
	m_fixFunc[ColKind::CAPSULE] = &ColliderBase::FixToCapsule;
}

CollideHitInfo ColliderBase::IsCollide(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto kind = colliderCol->GetKind();
	return (this->*m_collideFunc.at(kind))(pos, colliderCol, colliderPos, preInfo);
}

const Vec3& MyEngine::ColliderBase::FixNextPos(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto kind = colliderCol->GetKind();
	return (this->*m_fixFunc.at(kind))(pos, colliderCol, colliderPos, hitInfo);
}

CollideHitInfo ColliderBase::IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{false};
}

CollideHitInfo ColliderBase::IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{ false };
}

CollideHitInfo ColliderBase::IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{ false };
}

CollideHitInfo ColliderBase::IsCollideBox(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{ false };
}

CollideHitInfo ColliderBase::IsCollideLine(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{ false };
}

CollideHitInfo ColliderBase::IsCollideWall(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	assert(false && "実装されていない判定です");
	return CollideHitInfo{ false };
}

Vec3 ColliderBase::FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	assert(false && "実装されていない修正関数です");
	return colliderPos;
}

Vec3 ColliderBase::FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	assert(false && "実装されていない修正関数です");
	return colliderPos;
}
