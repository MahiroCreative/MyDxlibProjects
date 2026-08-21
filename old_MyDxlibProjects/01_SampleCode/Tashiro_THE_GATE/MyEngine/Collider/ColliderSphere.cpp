#include "Colliders.h"
#include "CollideHelp.h"

using namespace MyEngine;

ColliderSphere::ColliderSphere() :
	ColliderBase(ColKind::SPHERE),
	radius(0.0f)
{
}

CollideHitInfo ColliderSphere::IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return IsCollideSphereAndSphere(pos, colliderPos, this, col);
}

CollideHitInfo ColliderSphere::IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderCapsule*>(colliderCol);
	return IsCollideSphereAndCapsule(pos, colliderPos, this, col);
}

CollideHitInfo ColliderSphere::IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderTerrain*>(colliderCol);
	return IsCollideSphereAndTerrain(pos, colliderPos, this, col);
}

CollideHitInfo MyEngine::ColliderSphere::IsCollideBox(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderBox*>(colliderCol);
	return IsCollideSphereAndBox(pos, colliderPos, this, col, preInfo);
}

Vec3 ColliderSphere::FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return FixSphereToSphere(pos, colliderPos, this, col, hitInfo);
}
