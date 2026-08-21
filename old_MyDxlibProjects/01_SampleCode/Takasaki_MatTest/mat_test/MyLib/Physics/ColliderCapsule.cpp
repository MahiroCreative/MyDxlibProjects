#include "ColliderCapsule.h"

using namespace MyEngine;

ColliderCapsule::ColliderCapsule() :
	ColliderBase(ColliderBase::Kind::Sphere),
	radius(0.0f),
	height(0.0f)
{
}
