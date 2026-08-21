#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderSphere : public ColliderBase
	{
	public:
		ColliderSphere(ColideTag tag);

		float GetRadius()const { return radius; }

	public:
		float radius;
	};
}

