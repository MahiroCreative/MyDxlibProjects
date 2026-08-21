#pragma once
#include "ColliderBase.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Quaternion.h"

namespace MyEngine
{
	class ColliderTerrain : public ColliderBase
	{
	public:
		ColliderTerrain();
		~ColliderTerrain() {}

		void Init(const Vec3& center, const Quaternion& rot, const Vec3& norm, float width, float length, bool isTrigger);

		// MEMO:廃棄予定
		void Init(const Vec3& center, const Vec3& norm, float width, float length, bool isTrigger);
		
	protected:
		virtual CollideHitInfo IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;
		virtual CollideHitInfo IsCollideLine(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;

		Vec3 FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;
		Vec3 FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo) override;

	public:
		Vec3 norm;
		Vec3 front;
		Vec3 right;
		Quaternion rotation;
		Vec2 size;
	};
}

