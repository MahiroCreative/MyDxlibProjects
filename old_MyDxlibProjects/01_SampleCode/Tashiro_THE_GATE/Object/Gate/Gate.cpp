#include "Gate.h"
#include "Collider/ColliderCapsule.h"
#include "Collider/ColliderLine.h"
#include "Physics.h"
#include "File.h"
#include "SoundManager.h"
#include "Matrix4x4.h"
#include "MathHelp.h"
#include "Game.h"
#include "GateCamera.h"
#include "Object/Player.h"
#include "DebugDraw.h"

namespace
{
	const wchar_t* const FILE_GATE_A = L"M_Gate_A";
	const wchar_t* const FILE_GATE_B = L"M_Gate_B";
	const wchar_t* const FILE_DISSOLVE = L"I_Dissolve";
	const wchar_t* const FILE_WARP = L"S_Warp";
	// カプセルの長さ
	constexpr float CAPSULE_SIZE = 4.0f;
	// 半径
	constexpr float RADIUS = 2.0f;

}

Gate::Gate(const std::shared_ptr<GateCamera>& camera, const std::shared_ptr<GateCamera>& cameraFromPair, GateKind kind) :
	Object3DBase(Priority::STATIC, ObjectTag::GATE),
	m_updateFunc(&Gate::NormalUpdate),
	m_camera(camera),
	m_cameraFromPair(cameraFromPair),
	m_kind(kind),
	m_hitObjTag(ObjectTag::WALL)
{
	m_cbuffH = CreateShaderConstantBuffer(sizeof(UserData));
	m_userData = static_cast<UserData*>(GetBufferShaderConstantBuffer(m_cbuffH));
}

Gate::~Gate()
{
}

void Gate::Init(ObjectTag hitObjTag, const Vec3& pos, const Vec3& norm, const Vec3& dir, const std::weak_ptr<Player>& player)
{
	OnEntryPhysics();

	auto& fileMgr = FileManager::GetInstance();
	m_dissolve = fileMgr.Load(FILE_DISSOLVE);
	m_warpSe = fileMgr.Load(FILE_WARP);

	m_rigid.SetGravity(false);
	m_collider = std::dynamic_pointer_cast<MyEngine::ColliderCapsule>(CreateCollider(MyEngine::ColKind::CAPSULE));
	m_collider->isTrigger = true;
	m_collider->size = CAPSULE_SIZE;
	m_collider->radius = RADIUS;
	const wchar_t* id = FILE_GATE_A;
	if (m_kind == GateKind::Blue)
	{
		id = FILE_GATE_B;
	}
	LoadModel(id);
	m_scale *= 0.01f * CAPSULE_SIZE;

	m_player = player;

	ChangePos(hitObjTag, pos, norm, dir);
}

void Gate::ChangePos(ObjectTag hitObjTag, const Vec3& pos, const Vec3& norm, const Vec3& dir)
{
	m_hitObjTag = hitObjTag;
	m_norm = norm.GetNormalized();
	m_rigid.SetPos(pos);
	m_collider->dir = dir;
	m_right = Vec3::Cross(m_collider->dir, m_norm);
	m_updateFunc = &Gate::OpenUpdate;

	m_rotation = Quaternion::GetQuaternion(m_norm, Vec3::Front());
	// 壁に設置していない場合
	if (m_collider->dir.y < 1.0f)
	{
		m_rotation = Quaternion::GetQuaternion(Vec3::Front(), m_collider->dir) * m_rotation;
	}

	m_userData->center = pos;
	m_userData->dir = dir;
	m_userData->size = CAPSULE_SIZE;
	m_userData->radius = RADIUS * 0.95f;
	m_userData->frame = 0;

	if (m_pairGate.expired()) return;
	SetCameraPos();
}

void Gate::SetCameraPos()
{
	const auto& pos = m_rigid.GetPos();

	m_camera->SetTargetPos(pos);
	// 見る方向はペアの法線方向
	m_camera->SetBaseViewDir(m_norm);

	m_cameraFromPair->SetTargetPos(pos);
	// 見る方向は自身からペアの方向に向かうベクトルを法線方向に回転させて実行
	const auto& pair = m_pairGate.lock();
	auto dir = pair->GetPos() - pos;
	dir.Normalize();
	auto angle = std::acosf(Vec3::Dot(-m_norm, pair->GetNorm())) * Game::RAD_2_DEG;
	auto axis = Vec3::Cross(pair->GetNorm(), -m_norm);
	if (!axis.SqLength())
	{
		axis = m_collider->dir;
	}
	m_cameraFromPair->SetBaseViewDir(Quaternion::AngleAxis(angle, axis) * dir);
}

void Gate::Update()
{
	// TODO: 開く演出、閉じる演出、存在している間の演出を入れる
	// TODO: カメラの位置を対象のゲートからプレイヤーの位置に合わせて移動させてみて見た目を確認してい見る
	++m_userData->frame;
	m_userData->isCreate = !m_pairGate.expired();
	if (!m_pairGate.expired())
	{
		auto playerCameraFront = m_player.lock()->GetCamera()->GetInfo().front;
		m_camera->Update(m_collider->dir, playerCameraFront);

		m_cameraFromPair->Update(m_collider->dir, playerCameraFront);
	}
}

void Gate::DrawGatePos() const
{
	Object3DBase::Draw();

#ifdef _DEBUG
	auto& debug = MyEngine::DebugDraw::GetInstance();
	debug.DrawLine(m_rigid.GetPos(), m_rigid.GetPos() + m_norm * 5, 0x00ff00);
#endif
}

void Gate::DrawGate(int tex) const
{
	UpdateShaderConstantBuffer(m_cbuffH);
	SetShaderConstantBuffer(m_cbuffH, DX_SHADERTYPE_PIXEL, 4);

	if (tex < 0)
	{
		tex = MV1GetTextureGraphHandle(m_modelHandle, 0);
	}
	SetUseTextureToShader(1, tex);
	SetUseTextureToShader(2, m_dissolve->GetHandle());
	Object3DBase::Draw();
	SetUseTextureToShader(1, -1);
	SetUseTextureToShader(2, -1);

#ifdef _DEBUG
	auto& debug = MyEngine::DebugDraw::GetInstance();
	debug.DrawLine(m_rigid.GetPos(), m_rigid.GetPos() + m_norm * 5, 0x00ff00);
#endif
}

bool Gate::CheckWarp(const Vec3& targetPos)
{
	// ゲートから対象に向くベクトルとゲートの法線の内積が-になったらワープ可能
	const auto& gateTotarget = targetPos - (m_rigid.GetPos() + m_collider->center);
	auto dot = Vec3::Dot(m_norm, gateTotarget);
	auto a = dot * Game::RAD_2_DEG;
	return dot < 0.0f;
}

void Gate::OnWarp(const Vec3& targetPos, MyEngine::Rigidbody& targetRigid, bool isAddGravity)
{
	// FIXME: 処理変更
	// 
	// 位置修正について：
	// ワープからプレイヤーまでのベクトルをワープの右方向と上方向に射影する
	// 射影したことによって出たベクトルをワープ先の位置に足してあげる
	// これだと多分直あたりになるから法線方向に少しだけ押し出してあげる
	// 
	// 速度について：
	// 現在持っている速度を取得して
	// それを法線方向に加えてあげる
	// なお、法線方向が(0, 1, 0)からの角度が30～60くらいならその分傾けて射出するようにする
	// 
	// 

	// 音鳴らす
	SoundManager::GetInstance().PlaySe(m_warpSe->GetHandle());


	/* 場所変換 */
	const auto& pair = m_pairGate.lock();
	const auto& gateToTarget = targetPos - m_rigid.GetPos();

	const auto& rot = Quaternion::GetQuaternion(m_collider->dir, pair->GetCol()->dir);
	// ゲート上での右方向・上方向動いている大きさを取得
	const auto& right = Vec3::Projection(gateToTarget, m_right);
	const auto& up    = rot * Vec3::Projection(gateToTarget, m_collider->dir);
	// 修正位置の取得
	auto fixPos = pair->GetPos() + right + up + pair->GetNorm() * 0.1f;
	targetRigid.SetPos(fixPos);
	

	/* 速度変換 */
	// 現在の速度を取得
	auto vel = targetRigid.GetVelocity();
	auto velLen = vel.Length();
	vel.Normalize();

	auto rot1 = Quaternion::GetQuaternion(-m_norm, pair->GetNorm(), m_collider->dir);
	auto newVel = rot1 * vel * velLen;
	targetRigid.SetVelocity(newVel);


#if false
	// 自身の
	auto angleVel = std::acosf(Vec3::Dot(-m_norm, vel)) * Game::RAD_2_DEG;
	auto axisVel = Vec3::Cross(-m_norm, vel);
	if (axisVel.SqLength() == 0.0f)
	{
		axisVel = m_collider->dir;
	}

	auto rot1 = Quaternion::GetQuaternion(-m_norm, pair->GetNorm(), m_collider->dir);
	axisVel = rot1 * axisVel;

	auto dir = pair->GetNorm();
		
	velLen = std::min<float>(velLen, 3.0f);
	auto newVel = Quaternion::AngleAxis(angleVel, axisVel) * dir * velLen;

	targetRigid.SetVelocity(newVel);
#endif
}

void Gate::OpenUpdate()
{
}

void Gate::NormalUpdate()
{
}

void Gate::CloseUpdate()
{
}
