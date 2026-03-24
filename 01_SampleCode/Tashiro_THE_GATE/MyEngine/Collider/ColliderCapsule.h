#pragma once
#include "ColliderBase.h"
#include "Geometry/Vec3.h"

namespace MyEngine
{
	class ColliderCapsule : public ColliderBase
	{
	public:
		ColliderCapsule();

		void Init(const Vec3& center, const Vec3& dir, float radius, float size, bool isTrigger);

	protected:
		virtual CollideHitInfo IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideBox(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;

		Vec3 FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;
		Vec3 FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;

	public:
		Vec3 dir;
		float radius;
		float size;
	};
}