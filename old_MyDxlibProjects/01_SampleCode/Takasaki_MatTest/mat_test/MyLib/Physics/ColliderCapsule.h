#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderCapsule : public ColliderBase
	{
	public:
		ColliderCapsule();

	public:
		float radius;
		float height;
	};
}