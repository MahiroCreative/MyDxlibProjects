#include "EnemySphere.h"
#include "Enemy.h"
#include "ColliderSphere.h"
namespace
{
	/// <summary>
	/// 球の当たり判定半径
	/// </summary>
	constexpr float kSphereRadius = 10.0f;
	/// <summary>
	/// 球の生成間隔
	/// </summary>
	constexpr int kSphereCreateFrame = 50;
	const char* name = "Sphere";
}
EnemySphere::EnemySphere(MyEngine::Collidable::Priority priority, ObjectTag tag, std::shared_ptr<MyEngine::Collidable>enemy, Vec3 pos, Vec3 velocity, int moveNum, int color) : SphereBase(priority,tag,pos,velocity,color,kSphereRadius),
m_enemy(std::dynamic_pointer_cast<Enemy>(enemy))
{
	m_rigid->SetPos(pos);
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = m_radius;
	m_color = color;
	//moveNumによって挙動が変わるのかもしれない(実装するかわからん)
	//if (moveNum == 0)
	{
		m_moveUpdate = &EnemySphere::StraightUpdate;
	}
}

EnemySphere::~EnemySphere()
{
}

void EnemySphere::Init()
{
}

void EnemySphere::Update()
{
	(this->*m_moveUpdate)();
	DeleteJudge();
}

void EnemySphere::Draw()
{
	DrawSphere3D(m_rigid->GetPos().VGet(), kSphereRadius, 10, 0xffff00, m_color, false);
}

void EnemySphere::Hit()
{
	//m_isDeleteFlag = true;
}

void EnemySphere::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{

}

void EnemySphere::StraightUpdate()
{
	m_rigid->SetVelocity(VGet(m_velocity.x * 20, m_velocity.y * 20, m_velocity.z * 20));
}

void EnemySphere::DeleteJudge()
{
}