#include "Crystal.h"
#include"ColliderBox.h"

using namespace std;


Crystal::Crystal(Vec3 pos,Vec3 norm,Vec3 size) : Collidable(Priority::StageGimmick,ObjectTag::Crystal),
m_Hp(30)
{
	SetAntiGravity();
	m_size = size;
	m_rigid->SetPos(pos);
	auto col=AddCollider(MyEngine::ColliderBase::Kind::Box, MyEngine::ColliderBase::ColideTag::Body);
	auto box = dynamic_pointer_cast<MyEngine::ColliderBox>(col);
	box->norm = norm;
	box->size = m_size;
	box->rotation = Quaternion();
	AddThroughTag(ObjectTag::Stage);
}

Crystal::~Crystal()
{
}

void Crystal::Init()
{
}

void Crystal::Update()
{
	if (m_Hp < 0)
	{
		m_isDestroyFlag = true;
	}
}

void Crystal::Draw()
{
	DrawCube3D((m_rigid->GetPos() - m_size).VGet(), (m_rigid->GetPos() + m_size).VGet(), 0x00ffff, 0xffffff, true);
	DrawCube3D((m_rigid->GetPos() - m_size).VGet(), (m_rigid->GetPos() + m_size).VGet(), 0xffffff, 0xffffff, false);
}


void Crystal::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (colider->GetTag() == ObjectTag::PlayerBullet)
	{
		m_Hp -= 10;
	}
}
