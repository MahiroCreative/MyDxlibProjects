#include "ColliderSphere.h"

using namespace MyEngine;

ColliderSphere::ColliderSphere(ColideTag tag) :
	ColliderBase(ColliderBase::Kind::Sphere,tag),
	radius(0.0f)
{

}
