#pragma once
#include "ColliderBase.h"
#include"Vec3.h"
#include"Quaternion.h"
namespace MyEngine
{
	class ColliderBox : public ColliderBase
	{
	public:
		ColliderBox(ColideTag tag);

	public:
		Vec3 size;
		Quaternion rotation;
		Vec3 norm;
	};
}

