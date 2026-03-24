#include "Physics.h"
#include <cassert>
#include "Collidable.h"
#include "ColliderSphere.h"
#include "DebugDraw.h"

using namespace MyEngine;

namespace
{
	// 判定最大回数
	constexpr int CHECK_COUNT_MAX = 500;
}

Physics::Physics()
{
}

Physics& Physics::GetInstance()
{
	static Physics instance;
	return instance;
}

void Physics::Entry(const std::shared_ptr<Collidable>& collidable)
{
	bool isFound = std::find(m_collidables.begin(), m_collidables.end(), collidable) != m_collidables.end();
	//// 未登録なら追加
	//if (!isFound)
	//{
	//    m_collidables.emplace_back(collidable);
	//}
	//// 登録済みなら無視
	//else
	//{
	//    assert(false);
	//}

	// 未登録なら追加
	if (!isFound && collidable->GetTag() != ObjectTag::Stage)
	{
		m_collidables.emplace_back(collidable);
	}
	else if (!isFound)
	{
		m_stageCollidables.emplace_back(collidable);
		m_collidables.emplace_back(collidable);
	}
	// 登録済みなら無視
	else
	{
		assert(false);
	}
}

void Physics::Exit(const std::shared_ptr<Collidable>& collidable)
{
	auto it = std::find(m_collidables.begin(), m_collidables.end(), collidable);
	// 登録済みなら削除
	if (it!= m_collidables.end())
	{
		int index =static_cast<int>( distance(m_collidables.begin(), it));
		auto iterater = m_collidables.begin();
		for (int i = 0; i < index; i++)
		{
			iterater++;
		}
		m_collidables.erase(iterater);
	}
	// 未登録なら無視
	else
	{
		assert(false);
	}
}

void Physics::Update()
{
	m_preCollideInfo = m_newCollideInfo;
	m_newCollideInfo.clear();
	m_preTirrigerInfo = m_newTirrigerInfo;
	m_newTirrigerInfo.clear();
	m_onCollideInfo.clear();

	MoveNextPos();

	CheckCollide();

	CheckSendOnCollideInfo(m_preCollideInfo, m_newCollideInfo, false);
	CheckSendOnCollideInfo(m_preTirrigerInfo, m_newTirrigerInfo, true);

	FixPos();

	for (const auto& info : m_onCollideInfo)
	{
		if (static_cast<int>(info.send->m_tag) > 10|| static_cast<int>(info.send->m_tag)<0)continue;//応急処置済み:Exitしても履歴に残っているが参照できずに例外がスローされる
		if (static_cast<int>(info.own->m_tag) > 10 || static_cast<int>(info.own->m_tag) < 0)continue;//:上に同じ
		if (info.own->GetTag() == ObjectTag::ClearObject)
		{
			int a = 0;
		}
		OnCollideInfo(info.own, info.send, info.kind);
	}
}

void MyEngine::Physics::Draw()
{
	for (const auto& obj : m_collidables)
	{
		obj->Draw();
	}
}

void MyEngine::Physics::Clear()
{
	m_collidables.clear();
	m_stageCollidables.clear();

	m_onCollideInfo.clear();
	m_newCollideInfo.clear();
	m_preCollideInfo.clear();
	m_newTirrigerInfo.clear();
	m_preTirrigerInfo.clear();
}

/// <summary>
/// 物理からの移動を未来の座標に適用
/// </summary>
void MyEngine::Physics::MoveNextPos() const
{
	for (const auto& item : m_collidables)
	{
		auto& rigid = item->m_rigid;

		auto pos = rigid->GetPos();
		auto nextPos = pos + rigid->GetVelocity();

		rigid->SetNextPos(nextPos);

#ifdef _DEBUG
		auto& debug = DebugDraw::GetInstance();

		for (const auto& collider : item->m_colliders)
		{
			auto kind = collider->GetKind();
			if (kind == ColliderBase::Kind::Sphere)
			{
				auto sphereData = dynamic_cast<ColliderSphere*>(collider.get());
				DebugDraw::SphereInfo preInfo;
				preInfo.center = pos;
				preInfo.radius = sphereData->radius;
				preInfo.color = DebugDraw::COL_BEFOR;
				debug.DrawSphere(preInfo);
				DebugDraw::SphereInfo newInfo;
				newInfo.center = nextPos;
				newInfo.radius = sphereData->radius;
				newInfo.color = DebugDraw::COL_NEXT;
				debug.DrawSphere(newInfo);
			}
		}
#endif
	}
}

void MyEngine::Physics::CheckCollide()
{
	bool isCheck = true;
	int checkCount = 0;
	std::unordered_map<std::shared_ptr<Collidable>, std::list<std::shared_ptr<Collidable>>> newCollideInfo;
	while (isCheck)
	{
		isCheck = false;
		++checkCount;
		
		for (const auto& objA : m_collidables)
		{
			for (const auto& objB : m_collidables)
			{
				if (objA == objB) continue;
				// どちらかのスルー対象としてタグが入っていたら無視
				if (objA->IsThroughTarget(objB) || objB->IsThroughTarget(objA))continue;
				for (const auto& colA : objA->m_colliders)
				{
					for (const auto& colB : objB->m_colliders)
					{
						if (!IsCollide(objA->m_rigid, objB->m_rigid, colA, colB)) continue;

						bool isTrigger = colA->isTrigger || colB->isTrigger;

						if (isTrigger)
						{
							AddNewCollideInfo(objA, objB, m_newTirrigerInfo);
						}
						else
						{
							
							AddNewCollideInfo(objA, objB, m_newCollideInfo);
						}


						// Triggerの場合は位置補正はしない
						if (isTrigger)
						{
							isCheck = false;
							continue;
						}
						auto primary = objA;
						auto secondary = objB;
						auto primaryCollider = colA;
						auto secondaryCollider = colB;
						if (objA->m_priority < objB->m_priority)
						{
							primary = objB;
							secondary = objA;
							primaryCollider = colB;
							secondaryCollider = colA;
						}

						FixNextPos(primary->m_rigid, secondary->m_rigid, primaryCollider, secondaryCollider);
						// 位置補正をしたらもう一度初めから行う
						isCheck = true;
						break;
					}

					if (isCheck) break;
				}

				if (isCheck) break;
			}

			if (isCheck) break;
		}

		if (isCheck && checkCount > CHECK_COUNT_MAX)
		{
			//printfDx("規定数(%d)を超えました", CHECK_COUNT_MAX);
			break;
		}
	}
}

bool Physics::IsCollide(const std::shared_ptr<Rigidbody> rigidA, const std::shared_ptr<Rigidbody> rigidB, const std::shared_ptr<ColliderBase>& colliderA, const std::shared_ptr<ColliderBase>& colliderB) const
{

	bool isCollide = false;

	auto kindA = colliderA->GetKind();
	auto kindB = colliderB->GetKind();

	if (kindA == ColliderBase::Kind::Sphere && kindB == ColliderBase::Kind::Sphere)
	{
		auto sphereA = dynamic_cast<ColliderSphere*>(colliderA.get());
		auto sphereB = dynamic_cast<ColliderSphere*>(colliderB.get());

		auto aToB = rigidB->GetNextPos() - rigidA->GetNextPos();
		float sumRadius = sphereA->radius + sphereB->radius;
		isCollide = (aToB.SqLength() < sumRadius * sumRadius);
	}

	return isCollide;
}

void MyEngine::Physics::FixNextPos(const std::shared_ptr<Rigidbody> primaryRigid, std::shared_ptr<Rigidbody> secondaryRigid, const std::shared_ptr<ColliderBase>& primaryCollider, const std::shared_ptr<ColliderBase>& secondaryCollider)
{
	Vec3 fixedPos = secondaryRigid->GetNextPos();

	auto primaryKind = primaryCollider->GetKind();
	auto secondaryKind = secondaryCollider->GetKind();

	if (primaryKind == ColliderBase::Kind::Sphere)
	{
		if (secondaryKind == ColliderBase::Kind::Sphere)
		{
			auto primarySphere = dynamic_cast<ColliderSphere*>(primaryCollider.get());
			auto secondarySphere = dynamic_cast<ColliderSphere*>(secondaryCollider.get());

			// primaryからsecondaryへのベクトルを作成
			auto primaryToSecondary = secondaryRigid->GetNextPos() - primaryRigid->GetNextPos();
			// そのままだとちょうど当たる位置になるので少し余分に離す
			float  awayDist = primarySphere->radius + secondarySphere->radius + 0.0001f;
			// 長さを調整
			primaryToSecondary = primaryToSecondary.GetNormalized() * awayDist;
			// primaryからベクトル方向に押す
			fixedPos = primaryRigid->GetNextPos() + primaryToSecondary;
		}
	}

	secondaryRigid->SetNextPos(fixedPos);
}

void MyEngine::Physics::AddNewCollideInfo(std::shared_ptr<Collidable> objA, std::shared_ptr<Collidable> objB, SendCollideInfo& info)
{
	// Aが親として取得しているか
	bool isParentA = info.find(objA) != info.end();
	// Bが親として取得しているか
	bool isParentB = info.find(objB) != info.end();
	// AがBどちらかが取得している場合
	if (isParentA || isParentB)
	{
		std::shared_ptr<Collidable> parent = objA;
		std::shared_ptr<Collidable> child = objB;
		if (isParentB)
		{
			parent = objB;
			child = objA;
		}
		// 親が子を持っているか
		bool isChild = std::find(info[parent].begin(), info[parent].end(), child) != info[parent].end();
		// 子として持っていなければ追加
		if (!isChild)
		{
			info[parent].emplace_back(child);
		}
	}
	// どちらも取得していない場合
	else
	{
		// 普通に追加
		info[objA].emplace_back(objB);
	}
}

void MyEngine::Physics::CheckSendOnCollideInfo(SendCollideInfo& preSendInfo, SendCollideInfo& newSendInfo, bool isTrigger)
{
	for (auto& parent : newSendInfo)
	{
		// 以前の情報に親として登録されているか
		bool isPreParent = preSendInfo.find(parent.first) != preSendInfo.end();
		bool isAllEnter = true;

		for (auto& child : parent.second)
		{
			bool isPreChild = false;
			if (isPreParent)
			{
				// 以前の情報に子として登録されているか
				auto& preParent = preSendInfo[parent.first];
				isPreChild = std::find(preParent.begin(), preParent.end(), child) != preParent.end();
			}

			// 今回入ってきた場合はEnterを呼ぶ(子として登録されていない)
			if (!isPreChild)
			{
				if (isTrigger)
				{
					AddOnCollideInfo(parent.first, child, OnCollideInfoKind::TriggerEnter);
					AddOnCollideInfo(child, parent.first, OnCollideInfoKind::TriggerEnter);
				}
				else
				{
					AddOnCollideInfo(parent.first, child, OnCollideInfoKind::CollideEnter);
					AddOnCollideInfo(child, parent.first, OnCollideInfoKind::CollideEnter);
				}
			}

			// Stayは毎度呼ぶ
			if (isTrigger)
			{
				AddOnCollideInfo(parent.first, child, OnCollideInfoKind::TriggerStay);
				AddOnCollideInfo(child, parent.first, OnCollideInfoKind::TriggerStay);
			}
			else
			{
				AddOnCollideInfo(parent.first, child, OnCollideInfoKind::CollideStay);
				AddOnCollideInfo(child, parent.first, OnCollideInfoKind::CollideStay);
			}

			// 登録されていた情報を削除
			if (isPreChild)
			{
				preSendInfo[parent.first].remove(child);
			}
			// 全て登録されていなかったとする
			else
			{
				isAllEnter = false;
			}
		}

		// 全て登録されていたら親の情報も消す
		if (isAllEnter)
		{
			preSendInfo.erase(parent.first);
		}
	}

	// 残っている前情報からExistを呼ぶ(登録が残っている＝今回抜けた)
	for (auto& parent : preSendInfo)
	{
		for (auto& child : parent.second)
		{
			if (isTrigger)
			{
				AddOnCollideInfo(parent.first, child, OnCollideInfoKind::TriggerExit);
				AddOnCollideInfo(child, parent.first, OnCollideInfoKind::TriggerExit);
			}
			else
			{
				AddOnCollideInfo(parent.first, child, OnCollideInfoKind::CollideExit);
				AddOnCollideInfo(child, parent.first, OnCollideInfoKind::CollideExit);
			}
		}
	}
}

void MyEngine::Physics::AddOnCollideInfo(std::shared_ptr<Collidable> own, std::shared_ptr<Collidable> send, OnCollideInfoKind kind)
{
	OnCollideInfoData info;
	info.own = own;
	info.send = send;
	info.kind = kind;
	m_onCollideInfo.emplace_back(info);
}

void MyEngine::Physics::OnCollideInfo(std::shared_ptr<Collidable> own, std::shared_ptr<Collidable> send, OnCollideInfoKind kind)
{
	auto item=send;
	if (kind == OnCollideInfoKind::CollideEnter)
	{
		own->OnCollideEnter(item);
	}
	else if (kind == OnCollideInfoKind::CollideStay)
	{
		own->OnCollideStay(item);
	}
	else if (kind == OnCollideInfoKind::CollideExit)
	{
		own->OnCollideExit(item);
	}
	else if (kind == OnCollideInfoKind::TriggerEnter)
	{
		own->OnTriggerEnter(item);
	}
	else if (kind == OnCollideInfoKind::TriggerStay)
	{
		own->OnTriggerStay(item);
	}
	else if (kind == OnCollideInfoKind::TriggerExit)
	{
		own->OnTriggerExit(item);
	}
}

/// <summary>
/// 最終的な未来の座標から現在の座標に適用
/// </summary>
void Physics::FixPos() const
{
	for (const auto& item : m_collidables)
	{
		auto& rigid = item->m_rigid;

		rigid->SetPos(rigid->GetNextPos());

#ifdef _DEBUG
		auto& debug = DebugDraw::GetInstance();
		for (const auto& collider : item->m_colliders)
		{
			auto kind = collider->GetKind();
			if (kind == ColliderBase::Kind::Sphere)
			{
				auto sphereData = dynamic_cast<ColliderSphere*>(collider.get());
				DebugDraw::SphereInfo info;
				info.center = rigid->GetPos();
				info.radius = sphereData->radius;
				info.color = DebugDraw::COL_AFFTER;
				debug.DrawSphere(info);
			}
		}
#endif
	}

}
