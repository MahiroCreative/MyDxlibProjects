#include "TurretBullet.h"
#include "Collider/ColliderSphere.h"
#include "Object/Player.h"

namespace
{
	constexpr float RADIUS = 1.0f;
	constexpr float SPEED = 2.0f;
	constexpr int DAMAGE = 2;
}

TurretBullet::TurretBullet() :
	Object3DBase(Priority::LOW, ObjectTag::TURRET_BULLET),
	m_player(nullptr),
	m_isExist(true)
{
}

TurretBullet::~TurretBullet()
{
}

void TurretBullet::Init(const Vec3& pos, const Vec3& dir, Player* player)
{
	OnEntryPhysics();
	m_rigid.SetPos(pos);
	m_rigid.SetVelocity(dir * SPEED);
	m_rigid.SetGravity(false);

	auto collider = std::dynamic_pointer_cast<MyEngine::ColliderSphere>(CreateCollider(MyEngine::ColKind::SPHERE));
	collider->radius = RADIUS;
	collider->isTrigger = true;

	m_player = player;
}

void TurretBullet::OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerEnter(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	// タグがプレイヤーならダメージを与える
	if (tag == ObjectTag::PALYER)
	{
		m_player->OnDamage(DAMAGE);
	}
	// タグがプレイヤー・タレット・タレットの球でないなら球を終了させる
	else if (tag != ObjectTag::TURRET && tag != ObjectTag::TURRET_BULLET)
	{
		m_isExist = false;
	}
}
