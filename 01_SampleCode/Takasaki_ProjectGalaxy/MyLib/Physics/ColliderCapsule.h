#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderCapsule : public ColliderBase
	{
	public:
		ColliderCapsule(ColideTag tag);

	public:
		float radius;
		float height;
	};
}