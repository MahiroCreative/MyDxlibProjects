#include "ColliderBox.h"

MyEngine::ColliderBox::ColliderBox(Vec3 startPos, Vec3 endPos)
	:ColliderBase(ColliderBase::Kind::Box)
{
	m_startPos = startPos;
	m_endPos = endPos;
}
