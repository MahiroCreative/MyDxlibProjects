#include "Item.h"

#include"Physics.h"
#include"Quaternion.h"


namespace
{
	const char* name = "MaterialX";
}

Item::Item(Vec3 pos,bool antiGravity):Collidable(Priority::Static,ObjectTag::Item)
{
	SetAntiGravity(antiGravity);
	m_rigid->SetPos(pos);
}

Item::~Item()
{
	
}

void Item::Init()
{
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
	m_col = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	m_col->radius = 2.5f;
	
}

void Item::Update()
{
	//m_upVec = (m_rigid->GetPos() - m_nowPlanetPos).GetNormalized();
	//m_rigid->SetPos(m_rigid->GetPos());
	//m_upVec = m_upVec * sin(angle) ;
	//m_rigid->SetVelocity(m_upVec*0.1f);
}

void Item::Draw()
{
	Vec3 zero = { 0,0,0 };
	m_rigid->SetVelocity(Vec3(0, 0, 0));
	Vec3 offSetVec=GetCameraRightVector();
	offSetVec -= GetCameraUpVector();
	offSetVec *= 0.9;
	Quaternion myQ;
	angle+=0.05f;
	
	Vec3 front = GetCameraFrontVector();
	for (int i = 0; i < 3; i++)
	{
		myQ.SetQuaternion(offSetVec);
		myQ.SetMove(DX_PI_F*2/3*i+angle , front);
		Vec3 offSet=myQ.Move(offSetVec,zero);
		DrawSphere3D((m_rigid->GetPos() + offSet).VGet(), 1.0, 8, 0xff00ff, 0xff00ff, false);
	}

	DrawSphere3D(m_rigid->GetPos().VGet(), m_col->radius, 8, 0x00ff00, 0x00ff00, false);
}

void Item::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	
	if (colider->GetTag() == ObjectTag::Player)
	{
		m_deleteFlag = true;
	}
}

void Item::OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (colider->GetTag() == ObjectTag::Stage)	m_nowPlanetPos = colider->GetRigidbody()->GetPos();
}
