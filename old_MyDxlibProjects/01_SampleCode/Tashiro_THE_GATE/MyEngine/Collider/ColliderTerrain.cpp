#include "Colliders.h"
#include "CollideHelp.h"
#include "Common/Game.h"

using namespace MyEngine;

ColliderTerrain::ColliderTerrain() : 
	ColliderBase(ColKind::TERRAIN)
{
}

void ColliderTerrain::Init(const Vec3& center, const Quaternion& rot, const Vec3& norm, float width, float length, bool isTrigger)
{
	this->norm = norm;
	this->right = this->rotation * Vec3::Right();
	this->front = this->rotation * Vec3::Front();

	this->rotation = rot;

	this->size = Vec2(width, length);
	this->isTrigger = isTrigger;
}

void ColliderTerrain::Init(const Vec3& center, const Vec3& norm, float width, float length, bool isTrigger)
{
	auto N = norm.GetNormalized();
	this->norm = N;
	this->right = this->rotation * Vec3::Right();
	this->front = this->rotation * Vec3::Front();

	float angle = std::acosf(Vec3::Dot(N, Vec3::Up()));
	auto axis = Vec3::Cross(Vec3::Up(), N);
	this->rotation = Quaternion::AngleAxis(angle * Game::RAD_2_DEG, axis);

	this->size = Vec2(width, length);
	this->isTrigger = isTrigger;
}

CollideHitInfo ColliderTerrain::IsCollideSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return IsCollideSphereAndTerrain(colliderPos, pos, col, this);
}

CollideHitInfo ColliderTerrain::IsCollideCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderCapsule*>(colliderCol);
	return IsCollideCapsuleAndTerrain(colliderPos, pos, col, this);
}

CollideHitInfo MyEngine::ColliderTerrain::IsCollideLine(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const PreHitInfo& preInfo)
{
	auto col = dynamic_cast<ColliderLine*>(colliderCol);
	return IsCollideLineAndTerrain(colliderPos, pos, col, this);
}

Vec3 ColliderTerrain::FixToSphere(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto col = dynamic_cast<ColliderSphere*>(colliderCol);
	return FixTerrainToSphere(pos, colliderPos, this, col, hitInfo);
}

Vec3 ColliderTerrain::FixToCapsule(const Vec3& pos, ColliderBase* colliderCol, const Vec3& colliderPos, const CollideHitInfo& hitInfo)
{
	auto col = dynamic_cast<ColliderCapsule*>(colliderCol);
	return FixTerrainToCapsule(pos, colliderPos, this, col, hitInfo);
}
