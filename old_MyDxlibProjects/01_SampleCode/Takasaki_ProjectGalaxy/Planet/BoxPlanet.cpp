#include "BoxPlanet.h"
#include"ColliderBox.h"
namespace
{
	const Vec3 kBoxPlanetSize = Vec3(30, 50, 50);
}
BoxPlanet::BoxPlanet(Vec3 pos, int color)
{
	m_color = color;
	gravityPower = 0.3f;
	{
		AddCollider(MyEngine::ColliderBase::Kind::Box, MyEngine::ColliderBase::ColideTag::Body);//ここで入れたのは重力の影響範囲
		
		auto item = dynamic_pointer_cast<MyEngine::ColliderBox>(m_colliders.back());
		item->norm = Vec3(0, 1, 0);
		item->size = kBoxPlanetSize;
		item->rotation = Quaternion();
		norm = item->norm;
		size = item->size;
	}
	
	{
		AddCollider(MyEngine::ColliderBase::Kind::Box, MyEngine::ColliderBase::ColideTag::Body);//ここで入れたのは重力の影響範囲
		m_colliders.back()->isTrigger = true;
		auto item = dynamic_pointer_cast<MyEngine::ColliderBox>(m_colliders.back());
		item->norm = Vec3(0, 1, 0);
		item->size = Vec3(60, 100, 100);
		item->rotation = Quaternion();
		item->isTrigger = true;
	}
	AddThroughTag(ObjectTag::Stage);
	AddCollider(MyEngine::ColliderBase::Kind::Box, MyEngine::ColliderBase::ColideTag::Body);//マップの当たり判定
	m_rigid->SetPos(pos);

	m_pointLightHandle = CreatePointLightHandle(m_rigid->GetPos().VGet(), 500.0f, 0.0f, 0.002f, 0.0f);
}

BoxPlanet::~BoxPlanet()
{
}

void BoxPlanet::Init()
{
}

void BoxPlanet::Update()
{
}

void BoxPlanet::Draw()
{
	if (m_isSearch)
	{
		DrawCube3D((m_rigid->GetPos() - size).VGet(), (m_rigid->GetPos() + size).VGet(), 0xffffff, 0xffffff, false);

	}
	else
	{
		DrawCube3D((m_rigid->GetPos() - size).VGet(), (m_rigid->GetPos() + size).VGet(), 0x00ffff, 0xffffff, true);
	}
	
}

Vec3 BoxPlanet::GravityEffect(std::shared_ptr<Collidable> obj)
{
	Vec3 ans = obj->GetRigidbody()->GetVelocity() + norm*-1;
	return ans;
}

Vec3 BoxPlanet::GetNormVec(Vec3 pos)
{
	return norm;
}

void BoxPlanet::OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
}

void BoxPlanet::OnTriggerExit(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
}
