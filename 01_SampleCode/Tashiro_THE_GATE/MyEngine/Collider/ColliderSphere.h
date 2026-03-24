#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderSphere : public ColliderBase
	{
	public:
		ColliderSphere();

	protected:
		virtual CollideHitInfo IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideBox(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;

		Vec3 FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;

	public:
		float radius;
	};
}

