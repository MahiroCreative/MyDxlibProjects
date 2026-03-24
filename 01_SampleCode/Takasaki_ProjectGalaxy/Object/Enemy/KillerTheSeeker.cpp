#include "KillerTheSeeker.h"
#include"Killer.h"
#include"ColliderSphere.h"
#include"Physics.h"
#include"SoundManager.h"

namespace
{
	constexpr float kCollisionRadius = 200.f;

	/// <summary>
		/// 最大HP
		/// </summary>
	constexpr int kHp = 400;

	constexpr int kStartPosX = 200;
	constexpr int kStartPosY = 50;
	constexpr int kStartPosZ = 0;

	/// <summary>
	/// 足元からモデルの中心までの距離
	/// </summary>
	constexpr int kFootToCenter = 100;

	/// <summary>
	/// 攻撃クールタイム中の最低移動速度
	/// </summary>
	constexpr int kIdleSpeed = 20;
	/// <summary>
	/// 球の生成間隔
	/// </summary>
	constexpr int kSphereCreateFrame = 50;
	/// <summary>
	/// 再攻撃までのクールタイム
	/// </summary>
	constexpr int kAttackCoolDownTime = 300;

	/// <summary>
	/// ステージモデルの縦横サイズ/2
	/// </summary>
	constexpr int kStageSizeHalf = 200;

	const char* kShotSEhandlePath = "Shot.mp3";

	const char* kCounterHitSEhandlePath = "CounterHit.mp3";


	const char* name = "KillerTheSeeker";
}


KillerTheSeeker::KillerTheSeeker(Vec3 pos) :Enemy(-1, Priority::Static, ObjectTag::KillerTheSeeker),
m_Hp(kHp),
m_attackCoolDownCount(0),
m_centerToEnemyAngle(0),
m_radius(kCollisionRadius),
m_isSecondFase(false),
m_isHitFrame(0),
m_hitFrame(0)
{
	m_counterHitSEHandle = SoundManager::GetInstance().GetSoundData(kCounterHitSEhandlePath);
	SetCreate3DSoundFlag(true);
	m_shotSEHandle = SoundManager::GetInstance().GetSoundData(kShotSEhandlePath);
	SetCreate3DSoundFlag(false);
	Set3DRadiusSoundMem(1000, m_shotSEHandle);
	m_enemyUpdate = &KillerTheSeeker::IdleUpdate;
	m_rigid->SetPos(pos);
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = kCollisionRadius;
	m_moveShaftPos = m_rigid->GetPos();
	AddThroughTag(ObjectTag::Takobo);
	AddThroughTag(ObjectTag::Gorori);
	AddThroughTag(ObjectTag::WarpGate);
	m_color = 0x444444;
}

KillerTheSeeker::~KillerTheSeeker()
{
	m_sphere.erase(m_sphere.begin(), m_sphere.end());
}

void KillerTheSeeker::Init()
{
}

void KillerTheSeeker::Update()
{
	(this->*m_enemyUpdate)();

	for (auto& sphere : m_sphere)
	{
		if (m_sphere.size() == 0)return;
		sphere->Update();
	}
	if (m_isHitFrame)
	{
		m_hitFrame++;
		m_color = 0xff0000;
	}
	if (m_hitFrame > 30)
	{
		m_hitFrame = 0;
		m_isHitFrame = false;
	}
}

void KillerTheSeeker::DeleteManage()
{
	auto result = remove_if(m_sphere.begin(), m_sphere.end(), [this](const auto& sphere)
		{
			bool isOut = sphere->IsDelete() == true;
	if (isOut == true)
	{
		m_sphereNum--;
		MyEngine::Physics::GetInstance().Exit(sphere);
	}
	return isOut;
		});
	m_sphere.erase(result, m_sphere.end());
}

void KillerTheSeeker::Draw()
{
	DrawSphere3D(m_rigid->GetPos().VGet(), m_radius, 10, m_color, m_color, true);

	for (auto& sphere : m_sphere)
	{
		if (m_sphere.size() == 0)return;
		sphere->Draw();
	}

	//DrawBox(200, 700, 500 + m_Hp*3, 750, 0x00ff00, true);
}

void KillerTheSeeker::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (colider->GetTag() == ObjectTag::Stage)
	{
		m_nowPlanetPos = colider->GetRigidbody()->GetPos();
	}
	if (colider->GetTag() == ObjectTag::EnemyAttack)
	{
		auto attack = dynamic_pointer_cast<EnemySphere>(colider);
		if (attack->GetCounterFlag())
		{
			PlaySoundMem(m_counterHitSEHandle, DX_PLAYTYPE_BACK);
			attack->DeleteFlag();
			m_Hp -= 30;

		}
		m_isHitFrame = true;
	}
	if (m_Hp < 0 && !m_isSecondFase)
	{
		m_isSecondFase = true;

		m_attackCoolDownCount = 0;
		m_attackDir = GetAttackDir().GetNormalized();//オブジェクトに向かうベクトルを正規化したもの
		m_enemyUpdate = &KillerTheSeeker::AttackRollingUpdate;
		m_radius = 80;
		auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
		item->radius = 80;
		m_Hp = 80;
	}

}

Vec3 KillerTheSeeker::GetMyPos()
{
	return  VGet(m_rigid->GetPos().x, m_rigid->GetPos().y + kFootToCenter, m_rigid->GetPos().z);
}

void KillerTheSeeker::SetTarget(std::shared_ptr<Collidable> target)
{
	m_target = target;
}

void KillerTheSeeker::IdleUpdate()
{
	m_color = 0x444444;
	m_velocity = m_attackDir.GetNormalized();

	m_attackCoolDownCount++;

	if (m_attackCoolDownCount > kAttackCoolDownTime)
	{
		int attackState = GetRand(2);
		Vec3 norm = (m_rigid->GetPos() - m_nowPlanetPos).GetNormalized();
		Vec3 toTarget = ToVec(m_rigid->GetPos(), m_target->GetRigidbody()->GetPos());

		if (m_Hp < 300 && m_Hp >= 100)
		{
			m_color = 0xff0000;
			m_attackCoolDownCount = 0;
			m_attackDir = GetAttackDir();//オブジェクトに向かうベクトルを正規化したもの
			m_enemyUpdate = &KillerTheSeeker::AttackSphereUpdate;
		}
		else
		{
			switch (attackState)
			{
			case 0:
			{
				if (toTarget.Length() > 4000)break;
				m_attackCoolDownCount = 0;
				m_attackDir = GetAttackDir();//オブジェクトに向かうベクトルを正規化したもの
				m_enemyUpdate = &KillerTheSeeker::AttackSphereUpdate;
				break;
			}

			default:
				m_attackCoolDownCount = 250;
				break;
			}
		}
	}
}

void KillerTheSeeker::AttackSphereUpdate()
{
	m_rigid->SetVelocity(VGet(0, 0, 0));

	for (int i = 0; i < GetRand(5)+1; i++)
	{
		m_sphereNum++;

		m_createFrameCount = 0;
		Set3DPositionSoundMem(m_rigid->GetPos().VGet(), m_shotSEHandle);
		PlaySoundMem(m_shotSEHandle, DX_PLAYTYPE_BACK);
		m_sphere.push_back(std::make_shared<Killer>(Priority::Low, ObjectTag::EnemyAttack, shared_from_this(), m_target, GetMyPos()+m_upVec*i*2, m_attackDir, 1, 0xff0000));
		MyEngine::Physics::GetInstance().Entry(m_sphere.back());

	}	
	m_enemyUpdate = &KillerTheSeeker::IdleUpdate;
}

void KillerTheSeeker::AttackRollingUpdate()
{
	m_rigid->SetVelocity(m_velocity * 20);
	m_attackCount++;
	if (m_attackCount > 500)
	{
		m_velocity = m_attackDir.GetNormalized();
		m_attackCount = 0;
	}
}

Vec3 KillerTheSeeker::GetAttackDir() const
{
	Vec3 toVec = ToVec(m_rigid->GetPos(), m_target->GetRigidbody()->GetPos());
	Vec3 vec = norm(ToVec(m_rigid->GetPos(), m_target->GetRigidbody()->GetPos()));
	vec = VGet(vec.x * abs(toVec.x), vec.y * abs(toVec.y), vec.z * abs(toVec.z));
	return vec;
}

