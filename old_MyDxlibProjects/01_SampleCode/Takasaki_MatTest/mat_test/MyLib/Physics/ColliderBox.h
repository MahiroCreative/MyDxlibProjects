#pragma once
#include "ColliderBase.h"
#include"../Vec3.h"
namespace MyEngine
{
	class ColliderBox : public ColliderBase
	{
	public:
		ColliderBox(Vec3 startPos,Vec3 endPos);
	public:
		Vec3 m_startPos;
		Vec3 m_endPos;
	};
}

