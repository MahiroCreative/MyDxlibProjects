#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderLine : public ColliderBase
	{
	public:
		ColliderLine();

		virtual CollideHitInfo IsCollideTerrain(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo) override;

	public:
		Vec3 dir;
		float len;
	};
}

