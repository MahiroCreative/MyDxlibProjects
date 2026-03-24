#include "ColliderCapsule.h"

using namespace MyEngine;

ColliderCapsule::ColliderCapsule(ColideTag tag) :
	ColliderBase(ColliderBase::Kind::Sphere,tag),
	radius(0.0f),
	height(0.0f)
{
}
