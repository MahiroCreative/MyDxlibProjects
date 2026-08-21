#include "Gorori.h"
#include"ColliderSphere.h"
#include"Physics.h"
#include"SoundManager.h"
#include"ModelManager.h"

namespace
{
	constexpr float kCollisionRadius = 50.f;

	/// <summary>
		/// 最大HP
		/// </summary>
	constexpr int kHp = 20;

	constexpr int kStartPosX = 200;
	constexpr int kStartPosY = 50;
	constexpr int kStartPosZ = 0;

	/// <summary>
	/// 足元からモデルの中心までの距離
	/// </summary>
	constexpr int kFootToCenter = 30;

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

	const char* name = "gorori";
	const char* modelFileName = "Gorori.mv1";
}

/*プロトタイプ宣言*/
Vec3 ToVec(Vec3 a, Vec3 b);
Vec3 norm(Vec3 a);
float lerp(float start, float end, float t);

Gorori::Gorori(Vec3 pos,Vec3 velocity) :Enemy(-1, Priority::Static, ObjectTag::Gorori),
m_Hp(kHp),
m_attackCoolDownCount(0),
m_centerToEnemyAngle(0),
m_attackCount(0),
m_color(0xaaaa11),
m_modelHandle(ModelManager::GetInstance().GetModelData(modelFileName))
{
	m_enemyUpdate = &Gorori::IdleUpdate;
	m_rigid->SetPos(pos);
	AddCollider(MyEngine::ColliderBase::Kind::Sphere, MyEngine::ColliderBase::ColideTag::Body);
	auto item = dynamic_pointer_cast<MyEngine::ColliderSphere>(m_colliders.back());
	item->radius = kCollisionRadius;
	m_moveShaftPos = m_rigid->GetPos();
	AddThroughTag(ObjectTag::Gorori);
	AddThroughTag(ObjectTag::Takobo);
	AddThroughTag(ObjectTag::WarpGate);
	AddThroughTag(ObjectTag::EnemyAttack);
}

Gorori::~Gorori()
{

}

void Gorori::Init()
{
}

void Gorori::Update()
{
	(this->*m_enemyUpdate)();
}

void Gorori::SetMatrix()
{
	float angle = GetAngle(m_postUpVec, m_upVec);//前のフレームの上方向ベクトルと今の上方向ベクトル

	//printf("角度1=%f\n角度2=%f\n", angle, angle * 180.0f / 3.141592f);

	Vec3 axis = Cross(m_upVec, m_frontVec);//上方向ベクトルと進行方向ベクトルの外積から回転軸を生成
	axis.Normalize();//単位ベクトル化

	m_myQ = m_myQ.CreateRotationQuaternion(angle, axis) * m_myQ;//回転の掛け算

	auto rotatemat = m_myQ.ToMat();//クォータニオンから行列に変換

	//printf("x:%f,y:%f,z:%f\n", axis.x, axis.y, axis.z);


#ifdef _DEBUG
	//回転軸のデバッグ表示(紫)
	DrawLine3D(m_rigid->GetPos().VGet(), Vec3(m_rigid->GetPos() + axis * 100).VGet(), 0xff00ff);

	//上方向ベクトルのデバッグ表示(赤)
	DrawLine3D(m_rigid->GetPos().VGet(), Vec3(m_rigid->GetPos() + m_upVec * 100).VGet(), 0xff0000);

	//1フレーム前の上ベクトルの表示(暗赤)
	DrawLine3D(m_rigid->GetPos().VGet(), Vec3(m_rigid->GetPos() + m_postUpVec * 100).VGet(), 0xaa0000);

	//進行方向ベクトルのデバッグ表示(黄色)
	DrawLine3D(m_rigid->GetPos().VGet(), Vec3(m_rigid->GetPos() + m_frontVec * 100).VGet(), 0xffff00);

	//右側ベクトルのデバッグ表示(緑)
	DrawLine3D(m_rigid->GetPos().VGet(), Vec3(m_rigid->GetPos() + m_sideVec * 100).VGet(), 0x00ff00);

#endif 

	m_postUpVec = m_upVec;//上方向ベクトルを前のフレームの上方向ベクトルにする

	MV1SetRotationMatrix(m_modelHandle, rotatemat);//回転行列を反映

	MV1SetPosition(m_modelHandle, m_rigid->GetPos().VGet());
	auto modelMat = MV1GetMatrix(m_modelHandle);
}

void Gorori::Draw()
{
	DrawSphere3D(m_rigid->GetPos().VGet(), kCollisionRadius, 10,m_color, 0xff0000, true);
	MV1DrawModel(m_modelHandle);
}

void Gorori::OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag)
{
	if (colider->GetTag() == ObjectTag::Player)
	{
		m_Hp -= 20;
	}
}

Vec3 Gorori::GetMyPos()
{
	return  VGet(m_rigid->GetPos().x, m_rigid->GetPos().y + kFootToCenter, m_rigid->GetPos().z);;
}

void Gorori::SetTarget(std::shared_ptr<Collidable> target)
{
	m_target = target;
}

void Gorori::IdleUpdate()
{
	m_vec.x = 1;
	if (abs(m_rigid->GetPos().x - m_moveShaftPos.x) > 5)
	{
		m_vec.x *= -1;
	}

	m_rigid->SetVelocity(VGet(m_vec.x, 0, 0));

	m_attackCoolDownCount++;

	if ((m_rigid->GetPos() - m_target->GetRigidbody()->GetPos()).Length() > 100)
	{
		if (m_attackCoolDownCount > kAttackCoolDownTime)
		{
			int attackState = GetRand(1);
			switch (attackState)
			{
			case 0:
				m_attackCoolDownCount = 0;
				m_attackDir = GetAttackDir().GetNormalized();//オブジェクトに向かうベクトルを正規化したもの
				m_enemyUpdate = &Gorori::AttackUpdate;
				m_color = 0xff0000;
				break;
			default:
				m_attackCoolDownCount =0;
				break;
			}
		}
	}
}

void Gorori::AttackUpdate()
{
	m_rigid->SetVelocity(m_attackDir * 8);
	m_attackCount++;
	if (m_attackCount > 1000)
	{
		m_attackCount = 0;
		m_color = 0xaaaa11;
		m_enemyUpdate = &Gorori::IdleUpdate;
	}
}

Vec3 Gorori::GetAttackDir() const
{
	Vec3 vec = ToVec(m_rigid->GetPos(), m_target->GetRigidbody()->GetPos()).GetNormalized();
	
	return vec;
}
