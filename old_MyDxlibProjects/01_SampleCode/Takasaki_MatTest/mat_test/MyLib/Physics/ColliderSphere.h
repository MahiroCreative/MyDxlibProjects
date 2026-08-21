#pragma once
#include "ColliderBase.h"

namespace MyEngine
{
	class ColliderSphere : public ColliderBase
	{
	public:
		ColliderSphere();

	public:
		float radius;
	};
}

