#include "SphereBase.h"

SphereBase::SphereBase(MyEngine::Collidable::Priority priority, ObjectTag tag, Vec3 pos, Vec3 velocity, int color,float radius) : Collidable(priority,tag),
m_radius(radius),
m_velocity(velocity),
m_counterFlag(false)
{
}

SphereBase::~SphereBase()
{
}
