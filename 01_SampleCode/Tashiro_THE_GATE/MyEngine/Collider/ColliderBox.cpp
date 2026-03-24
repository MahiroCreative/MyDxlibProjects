#include "Colliders.h"
#include "CollideHelp.h"

using namespace MyEngine;

ColliderBox::ColliderBox() :
	ColliderBase(ColKind::BOX)
{
}

CollideHitInfo ColliderBox::IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return IsCollideSphereAndBox(colliderPos, pos, col, this, preInfo);
}

CollideHitInfo MyEngine::ColliderBox::IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderCapsule*>(colliderCol);
	return IsCollideCapsuleAndBox(colliderPos, pos, col, this, preInfo);
}

Vec3 ColliderBox::FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return FixBoxToSphere(pos, colliderPos, this, col, hitInfo);
}

Vec3 MyEngine::ColliderBox::FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto col = dynamic_cast<ColliderCapsule*>(colliderCol);
	return FixBoxToCapsule(pos, colliderPos, this, col, hitInfo);
}
