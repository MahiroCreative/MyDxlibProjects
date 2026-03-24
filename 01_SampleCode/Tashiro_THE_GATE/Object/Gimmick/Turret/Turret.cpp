#include "Turret.h"
#include "Collider/ColliderSphere.h"
#include "AnimController.h"
#include "Easing.h"
#include "TurretBullet.h"
#include "Object/Player.h"


namespace
{
	constexpr float RADIUS = 3.0f;

	// 動く速度
	constexpr float MOVE_SPEED = 0.01f;
	// 動く範囲(縦のみ)
	constexpr float MOVE_SIZE = 2.0f;

	// 見つける範囲
	constexpr float DISCOVERY_RANGE = 1.0f - 60.0f / 180.0f;
	// 見つける距離
	constexpr float DISCOVERY_LEN = 50.0f;

	// 攻撃範囲
	constexpr float ATTACK_RANGE = 1.0f - 10.0f / 180.0f;
	// 攻撃間隔
	constexpr float ATTACK_INTERVAL = 10;
}

Turret::Turret() :
	Object3DBase(Priority::LOW, ObjectTag::TURRET),
	m_player(nullptr),
	m_attackFrame(0),
	m_isAttack(false)
{
}

Turret::~Turret()
{
}

void Turret::Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity)
{
	Object3DBase::Init(pos, scale, rot, list, isGravity);

//	m_anim = std::make_shared<AnimController>();
//	m_anim->Init(m_modelHandle, L"DroneGuard|WakeUp");
}

void Turret::Init(const Vec3& dir, Player* player)
{
	m_baseDir = dir;
	m_lookDir = dir;

	m_player = player;
}

void Turret::Update()
{
//	m_anim->Update();
	LookUpdate();
	AttackUpdate();
	m_bulletList.remove_if(
		[](const auto& bullet)
		{
			auto isEnd = !bullet->IsExist();
			if (isEnd)
			{
				bullet->End();
			}
			return isEnd;
		}
	);
}

void Turret::Draw() const
{
	Object3DBase::Draw();

	auto s = m_rigid.GetPos() + m_collider.at(0)->center;
	auto e = s + m_baseDir * 3;
	DrawLine3D(s.VGet(), e.VGet(), 0x0000ff);
	e = s + m_lookDir * 3;
	DrawLine3D(s.VGet(), e.VGet(), 0x00ff00);
}

void Turret::LookUpdate()
{
	auto toPlayer = m_player->GetPos() - (m_rigid.GetPos() + m_collider.at(0)->center);
	// プレイヤーまでの距離が遠ければ終了
	if (toPlayer.SqLength() > DISCOVERY_LEN * DISCOVERY_LEN)
	{
		m_isAttack = false;
		return;
	}
	toPlayer.Normalize();
	auto dot = Vec3::Dot(toPlayer, m_baseDir);
	// ベースとなるベクトルとプレイヤーまでのベクトルの内積が一定値以上ならそちらの方向を向く
	if (dot > DISCOVERY_RANGE)
	{
		// 少しずつプレイヤーの方向に向いていく
		m_lookDir = Easing::Slerp(m_lookDir, toPlayer, 0.05f);
		// 現在の向いている方向とプレイヤーへの方向の内積が一定値以上なら攻撃可能にする
		dot = Vec3::Dot(m_lookDir, toPlayer);
		m_isAttack = dot > ATTACK_RANGE;
	}
	// 違うなら元の方向に戻る
	else
	{
		m_lookDir = Easing::Slerp(m_lookDir, m_baseDir, 0.05f);
		m_isAttack = false;
	}
}

void Turret::AttackUpdate()
{
	// アタックフレームを増やす
	++m_attackFrame;

	// 攻撃しないならここで処理終了
	if (!m_isAttack) return;

	// 一定フレームを超えたら攻撃
	if (m_attackFrame > ATTACK_INTERVAL)
	{
		OnAttack();
		m_attackFrame = 0;
	}
}

void Turret::OnAttack()
{
	auto bullet = std::make_shared<TurretBullet>();
	bullet->Init(m_rigid.GetPos() + m_collider.at(0)->center, m_lookDir, m_player);
	m_bulletList.emplace_back(bullet);
}
