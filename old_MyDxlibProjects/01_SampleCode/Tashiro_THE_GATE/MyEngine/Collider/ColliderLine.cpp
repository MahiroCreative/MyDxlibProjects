#include "Colliders.h"
#include "CollideHelp.h"

using namespace MyEngine;

ColliderLine::ColliderLine() :
	ColliderBase(ColKind::LINE),
	len(0.0f)
{
}

CollideHitInfo ColliderLine::IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderTerrain*>(colliderCol);
	return IsCollideLineAndTerrain(pos, colliderPos, this, col);
}
