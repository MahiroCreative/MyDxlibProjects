#pragma once
#include "Object3DBase.h"

class Glass : public Object3DBase
{
public:
	Glass();
	~Glass();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
};

