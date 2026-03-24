#include "GateManager.h"
#include <cassert>
#include "Gate.h"
#include "GateBullet.h"
#include "GateCamera.h"
#include "CameraManager.h"
#include "StageDataManager.h"
#include "Physics.h"
#include "Collider/CollideHelp.h"
#include "Collider/ColliderCapsule.h"
#include "Collider/ColliderBox.h"

namespace
{
	// 判定数
	constexpr int CHECK_NUM = 4;

	// ゲートのカプセルの長さ
	constexpr float GATE_CAPSULE_SIZE = 4.0f;
	// ゲートの半径
	constexpr float GATE_RADIUS = 2.0f;
}

GateManager::GateManager(const std::shared_ptr<CameraManager>& cameraMgr) :
	m_cameraMgr(cameraMgr),
	m_gateOrange(nullptr),
	m_gateBlue(nullptr),
	m_lastShotKind(GateKind::Blue)
{
	m_isCreate[GateKind::Orange] = false;
	m_isCreate[GateKind::Blue] = false;
}

GateManager::~GateManager()
{
}

void GateManager::End()
{
	if (m_gateOrange)
	{
		m_gateOrange->End();
		m_gateOrange = nullptr;
	}
	if (m_gateBlue)
	{
		m_gateBlue->End();
		m_gateBlue = nullptr;
	}
}

void GateManager::Update()
{
	if (m_isCreate.at(GateKind::Blue)) m_gateBlue->Update();
	if (m_isCreate.at(GateKind::Orange)) m_gateOrange->Update();
	for (auto& isChange : m_isChange)
	{
		isChange.second = false;
	}
	for (auto& bullet : m_bulletList)
	{
		bullet->Update();
	}
	m_bulletList.remove_if(
		[](const auto& bullet)
		{
			if (bullet->IsBreak())
			{
				bullet->End();
				return true;
			}
			return false;
		});
}

void GateManager::DrawGatePos() const
{
	if (m_isCreate.at(GateKind::Orange)) m_gateOrange->DrawGatePos();
	if (m_isCreate.at(GateKind::Blue)) m_gateBlue->DrawGatePos();
}

void GateManager::DrawGate(int texA, int texB) const
{
	// TODO: 対象のゲートの位置からプレイヤーまでの距離を求めてのカメラに変更する
	if (m_isCreate.at(GateKind::Orange)) m_gateOrange->DrawGate(texA);
	if (m_isCreate.at(GateKind::Blue)) m_gateBlue->DrawGate(texB);
}

void GateManager::Restart()
{
	// ゲートの破棄
	m_gateOrange = nullptr;
	m_gateBlue = nullptr;
	// 未生成に更新
	m_isCreate[GateKind::Orange] = false;
	m_isCreate[GateKind::Blue] = false;
	m_isChange[GateKind::Orange] = false;
	m_isChange[GateKind::Blue] = false;
	// ゲート弾を削除
	m_bulletList.clear();
}

void GateManager::AddBullet(std::shared_ptr<GateBullet> bullet)
{
	m_lastShotKind = bullet->GetKind();
	m_bulletList.emplace_back(bullet);
}

void GateManager::CreateGate(GateKind kind, MyEngine::Collidable* collider, const MyEngine::CollideHitInfo& hitInfo, const Vec3& dir)
{
	Vec3 pos = hitInfo.hitPos;
	auto col = new MyEngine::ColliderCapsule();
	col->dir = dir;
	col->radius = GATE_RADIUS;
	col->size = GATE_CAPSULE_SIZE;

	std::shared_ptr<Gate> gate;
	if (kind == GateKind::Orange)
	{
		gate = m_gateOrange;
	}
	else if (kind == GateKind::Blue)
	{
		gate = m_gateBlue;
	}
	else
	{
		assert(false && "存在しないゲートの種類です");
	}
	
	
	int m_checkCount = 0;
	bool isCreate = false;
	MyEngine::PreHitInfo info;
	info.info.fixDir = dir;
	info.col = gate;
	while (m_checkCount < CHECK_NUM)
	{
		++m_checkCount;
		// 壁との判定
		if (CheckCreateGateHit(pos, info, col, collider, ObjectTag::WALL)) continue;
		// 床との判定
		if (CheckCreateGateHit(pos, info, col, collider, ObjectTag::FLOOR)) continue;
		isCreate = true;
		break;
	}

	delete col;
	// 生成できない場合
	if (isCreate)
	{
		if (kind == GateKind::Orange)
		{
			CreateGate(m_gateOrange, kind, collider->GetTag(), pos, hitInfo.fixDir, dir);
		}
		else if (kind == GateKind::Blue)
		{
			CreateGate(m_gateBlue, kind, collider->GetTag(), pos, hitInfo.fixDir, dir);
		}
	}
	else
	{
		// TODO: エフェクトと失敗SEを流すようにする
	}
}

void GateManager::CreateGateOnTerrain(const wchar_t* const stageName)
{
	auto& stageDataMgr = StageDataManager::GetInstance();
	// 片手ステージでないなら終了
	if (!stageDataMgr.IsOneHand(stageName)) return;

	// 必要データ取得
	const auto& tag = stageDataMgr.GetGateObjectTag(stageName);
	const auto& pos = stageDataMgr.GetGatePos(stageName);
	const auto& norm = stageDataMgr.GetGateNorm(stageName);
	const auto& dir = stageDataMgr.GetGateDir(stageName);
	CreateGate(m_gateBlue, GateKind::Blue, tag, pos, norm, dir);
}

std::shared_ptr<Gate> GateManager::GetPairGate(GateKind kind) const
{
	if (kind == GateKind::Orange)
	{
		return m_gateBlue;
	}
	else if (kind == GateKind::Blue)
	{
		return m_gateOrange;
	}
	else
	{
		assert(false && "存在しないゲートの種類です");
		return m_gateOrange;
	}
}

bool GateManager::CheckCreateGateHit(Vec3& pos, MyEngine::PreHitInfo& preInfo, MyEngine::ColliderCapsule* col, MyEngine::Collidable* collider, ObjectTag tag)
{
	auto& physics = MyEngine::Physics::GetInstance();
	// 判定
	auto res = physics.GetHitObject(pos, col, tag, preInfo, true, collider);
	// 当たったものが存在しなければ終了
	if (res.empty()) return false;
	// 位置の修正
	auto& info = res[0];
	pos = MyEngine::FixBoxToCapsule(info.col->GetPos(), pos, dynamic_cast<MyEngine::ColliderBox*>(info.col->GetColliderData(0)), col, info.hitInfo);
	return true;
}

void GateManager::CreateGate(std::shared_ptr<Gate>& gate, GateKind kind, ObjectTag hitTag, const Vec3& pos, const Vec3& norm, const Vec3& dir)
{
	auto pairGate = GetPairGate(kind);
	m_isChange[kind] = true;
	// ゲートを既に作成している場合
	if (m_isCreate[kind])
	{
		gate->ChangePos(hitTag, pos, norm, dir);
	}
	// ゲートをまだ作成していない場合
	else
	{
		CameraKind kind1 = CameraKind::GATE_B;
		CameraKind kind2 = CameraKind::GATE_B_FROM_A;
		if (kind == GateKind::Blue)
		{
			kind1 = CameraKind::GATE_A;
			kind2 = CameraKind::GATE_A_FROM_B;
		}
		gate = std::make_shared<Gate>(std::dynamic_pointer_cast<GateCamera>(m_cameraMgr->GetCamera(kind1)), std::dynamic_pointer_cast<GateCamera>(m_cameraMgr->GetCamera(kind2)), kind);
		gate->Init(hitTag, pos, norm, dir, m_player);
		if (pairGate)
		{
			gate->SetPairGate(pairGate);
			pairGate->SetPairGate(gate);

			gate->SetCameraPos();
			pairGate->SetCameraPos();
		}
	}

	m_isCreate[kind] = true;
}
