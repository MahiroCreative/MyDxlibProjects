#include "LaserBullet.h"
#include "Physics.h"
#include "MathHelp.h"
#include "LaserLaunchPad.h"
#include "Collider/ColliderSphere.h"
#include "Collider/ColliderBox.h"
#include "Object/Gate/Gate.h"
#include "Object/Gate/GateManager.h"

namespace
{
	constexpr float MOVE_SPEED = 0.5f;
	constexpr int REFRECTION_SUB_TIME = 5;
}

LaserBullet::LaserBullet(LaserLaunchPad* launcPad, const GateManager* gateMgr) :
	Object3DBase(Priority::LOW, ObjectTag::LASER_BULLET),
	m_launchPad(launcPad),
	m_gateMgr(gateMgr),
	m_time(0),
	m_isAddThroughTag(false),
	m_isWarp(false)
{
}

LaserBullet::~LaserBullet()
{
}

void LaserBullet::Init(const Vec3& pos, const Vec3& dir, int existTime)
{
	OnEntryPhysics();
	m_rigid.SetGravity(false);
	m_rigid.SetPos(pos);
	m_rigid.SetVelocity(dir.GetNormalized() * MOVE_SPEED);
	
	auto col = std::dynamic_pointer_cast<MyEngine::ColliderSphere>(CreateCollider(MyEngine::ColKind::SPHERE));
	col->isTrigger = false;
	col->radius = 2.0f;

	m_time = existTime;

	m_throughTag.push_back(ObjectTag::PALYER);
	m_throughTag.push_back(ObjectTag::GATE_BULLET);
	m_throughTag.push_back(ObjectTag::TURRET);
	m_throughTag.push_back(ObjectTag::TURRET_BULLET);
	m_throughTag.push_back(ObjectTag::LASER_LAUNCH_PAD);
}

void LaserBullet::Update()
{
	if (m_gateMgr->IsCreateBothGates())
	{
		auto& physics = MyEngine::Physics::GetInstance();
		auto res = physics.GetHitObject(m_rigid.GetPos(), GetColliderData(0), ObjectTag::GATE);
		if (res.empty() && m_isAddThroughTag)
		{
			m_isAddThroughTag = false;
			m_throughTag.pop_back();
		}
		else if (!res.empty() && !m_isAddThroughTag)
		{
			m_isAddThroughTag = true;
			auto gate = dynamic_cast<Gate*>(res[0].col);
			m_throughTag.emplace_back(gate->GetHitObjectTag());
		}
	}

	--m_time;
	// 制限時間がたったら消滅させる
	if (m_time < 0)
	{
		m_launchPad->DestoryBullet();
	}
}

void LaserBullet::OnCollideEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnCollideEnter(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::WALL || tag == ObjectTag::FLOOR || tag == ObjectTag::ROOF || tag == ObjectTag::NO_GATE_FLOOR || tag == ObjectTag::NO_GATE_WALL || tag == ObjectTag::NO_GATE_ROOF)
	{
		auto col = dynamic_cast<MyEngine::ColliderBox*>(colider->GetColliderData(sendIndex));

		// 反射
		auto dir = m_rigid.GetDir();
		const auto& nearPos = GetNearestPtOnBox(m_rigid.GetPos(), colider->GetPos() + col->center, col->size, col->rotation);
		const auto& norm = (m_rigid.GetPos() - nearPos).GetNormalized();
		dir = Vec3::Reflection(dir, norm);
		m_rigid.SetVelocity(dir * MOVE_SPEED);

		// 制限時間を一定時間減らす
		m_time -= REFRECTION_SUB_TIME;
	}
}

void LaserBullet::OnTriggerEnter(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerEnter(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::LASER_CATCHER)
	{
		m_launchPad->OnClear();
		m_launchPad->DestoryBullet();
	}
}

void LaserBullet::OnTriggerStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo)
{
	MyEngine::Collidable::OnTriggerStay(colider, selfIndex, sendIndex, hitInfo);

	auto tag = colider->GetTag();
	if (tag == ObjectTag::GATE)
	{
		if (!m_gateMgr->IsCreateBothGates()) return;
		auto gate = dynamic_cast<Gate*>(colider);
		// ワープ処理
		if (gate->CheckWarp(m_rigid.GetPos()))
		{
			gate->OnWarp(m_rigid.GetPos(), m_rigid, false);
		}
	}
}

