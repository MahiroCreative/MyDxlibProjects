#include "PlayerSphere.h"
#include "ColliderSphere.h"
#include "PlayerSphere.h"
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
PlayerSphere::PlayerSphere(MyEngine::Collidable::Priority priority, ObjectTag tag, Vec3 pos, Vec3 velocity, int moveNum, int color) : SphereBase(priority, tag, pos, velocity, color, kSphereRadius)
{
	m_rigid->SetPos(pos);
	AddCollider(MyEngine::ColliderBase::Kind::Sphere);
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = m_radius;
	m_color = color;
	SetAntiGravity(true);
	//moveNumによって挙動が変わるのかもしれない(実装するかわからん)
	//if (moveNum == 0)
	{
		m_moveUpdate = &PlayerSphere::StraightUpdate;
	}
}

PlayerSphere::~PlayerSphere()
{
}

void PlayerSphere::Init()
{
}

void PlayerSphere::Update()
{
	(this->*m_moveUpdate)();
}

void PlayerSphere::Draw()
{
	DrawSphere3D(m_rigid->GetPos().VGet(), kSphereRadius, 10, 0xffff00, m_color, false);
}

void PlayerSphere::Hit()
{
	//m_isDeleteFlag = true;
}

void PlayerSphere::OnCollideEnter(std::shared_ptr<Collidable> colider)
{
}

void PlayerSphere::StraightUpdate()
{
	m_rigid->SetVelocity(VGet(m_velocity.x*20, m_velocity.y*20, m_velocity.z*20));
}