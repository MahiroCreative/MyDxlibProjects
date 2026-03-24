#include "Invisible.h"
#include"ColliderSphere.h"

namespace
{
	constexpr float kRadius = 50.f;
	constexpr int kChaseSpeed = 2.f;

}
Invisible::Invisible(Vec3 pos) : Collidable(Priority::Low,ObjectTag::InvisibleObject)
{
	m_rigid->SetPos(pos);
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = kRadius;
}

Invisible::~Invisible()
{
}

void Invisible::Init()
{
}

void Invisible::Update()
{
}

void Invisible::Draw()
{
	SetDrawBlendMode(DX_BLENDMODE_MULA, 100);
	DrawSphere3D(m_rigid->GetPos().VGet(), kRadius, 8, 0xffaa00, 0xffffff, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
