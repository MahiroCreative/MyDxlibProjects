#pragma once
#include "ColliderBase.h"
#include "Geometry/Vec3.h"
#include "Geometry/Quaternion.h"

namespace MyEngine
{
	class ColliderBox : public ColliderBase
	{
	public:
		ColliderBox();

		virtual CollideHitInfo IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		Vec3 FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;
		Vec3 FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;

	public:
		Vec3 size;
		Quaternion rotation;
		Vec3 norm;
	};
}

