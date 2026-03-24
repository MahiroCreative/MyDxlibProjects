#include "Physics.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include"CollisionUtil.h"
#include "Collidable.h"
#include "ColliderSphere.h"
#include"ColliderBox.h"
#include "DebugDraw.h"
#include"Planet.h"

using namespace MyEngine;

namespace
{
	// 判定最大回数
	constexpr int CHECK_COUNT_MAX = 500;
}

Vec3 closestPointOnCubeAndSphere(const Vec3& cubeCenter, Vec3 size, const Vec3& sphereCenter, double sphereRadius);

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
	if (it != m_collidables.end())
	{
		int index = static_cast<int>(distance(m_collidables.begin(), it));
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
	Gravity();

	for (auto& item : m_collidables)
	{
		for (auto& col : item->m_colliders)
		{
			col->SetNowOnHit(false);
		}
		item->Update();
	}
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
		if (static_cast<int>(info.send->col->m_tag) > static_cast<int>(ObjectTag::End) || static_cast<int>(info.send->col->m_tag) < 0)continue;//応急処置済み:Exitしても履歴に残っているが参照できずに例外がスローされる
		if (static_cast<int>(info.own->col->m_tag) > static_cast<int>(ObjectTag::End) || static_cast<int>(info.own->col->m_tag) < 0)continue;//:上に同じ

		OnCollideInfo(info.own, info.send, info.kind);
	}

	auto result = remove_if(m_collidables.begin(), m_collidables.end(), [this](const auto& collision)
		{
			bool isOut = collision->IsDestroy() == true;
	return isOut;
		});
	m_collidables.erase(result, m_collidables.end());
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

void MyEngine::Physics::Gravity()
{
	for (const auto& item : m_stageCollidables)
	{
		for (const auto& obj : m_collidables)
		{
			if (obj->GetTag() != ObjectTag::Stage)
			{
				auto planet = dynamic_cast<Planet*>(item.get());
				for (const auto& col : item->m_colliders)
				{
					if (col->isTrigger == true)
					{
						int colIndex = 0;
						for (const auto& objCol : obj->m_colliders)
						{
							if (IsCollide(item->m_rigid, obj->m_rigid, col, objCol).isHit)
							{
								planet->OnTriggerEnter(obj, col->GetTag(), objCol->GetTag());
								obj->m_rigid->SetVelocity(planet->GravityEffect(obj));
							}
							colIndex++;
						}

					}
				}

			}

		}

	}
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
						CollideHitInfo info = IsCollide(objA->m_rigid, objB->m_rigid, colA, colB);

						if (!info.isHit) continue;

						std::shared_ptr<ColideInfo> infoA;
						infoA = std::make_shared<ColideInfo>();
						infoA->col = objA; infoA->tag = colA->GetTag();

						std::shared_ptr<ColideInfo> infoB;
						infoB = std::make_shared<ColideInfo>();
						infoB->col = objB; infoB->tag = colB->GetTag();
						bool isTrigger = colA->isTrigger || colB->isTrigger;

						if (isTrigger)
						{
							AddNewCollideInfo(infoA, infoB, m_newTirrigerInfo);
						}
						else
						{
							AddNewCollideInfo(infoA, infoB, m_newCollideInfo);
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

						colA->SetNowOnHit(true);
						colB->SetNowOnHit(true);
						FixNextPos(primary->m_rigid, secondary->m_rigid, primaryCollider, secondaryCollider, info);
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

MyEngine::Physics::CollideHitInfo Physics::IsCollide(const std::shared_ptr<Rigidbody> rigidA, const std::shared_ptr<Rigidbody> rigidB, const std::shared_ptr<ColliderBase>& colliderA, const std::shared_ptr<ColliderBase>& colliderB) const
{

	CollideHitInfo info;

	auto kindA = colliderA->GetKind();
	auto kindB = colliderB->GetKind();

	if (kindA == ColliderBase::Kind::Sphere && kindB == ColliderBase::Kind::Sphere)
	{
		auto sphereA = dynamic_cast<ColliderSphere*>(colliderA.get());
		auto sphereB = dynamic_cast<ColliderSphere*>(colliderB.get());

		auto aToB = (rigidB->GetNextPos() + colliderB->GetShift()) - (rigidA->GetNextPos() + colliderA->GetShift());
		float sumRadius = sphereA->radius + sphereB->radius;
		info.isHit = (aToB.SqLength() < sumRadius * sumRadius);
	}
	if (kindA == ColliderBase::Kind::Sphere && kindB == ColliderBase::Kind::Box)
	{
		auto SphereA = dynamic_cast<ColliderSphere*>(colliderA.get());
		auto BoxB = dynamic_cast<ColliderBox*>(colliderB.get());

		auto spherePos = rigidA->GetPos() + colliderA->GetShift();
		auto boxPos = rigidB->GetPos() + colliderB->GetShift();

		// ボックスの中心から円の中心までのベクトルを作成
		auto boxToSphere = spherePos - boxPos;
		auto radius = SphereA->radius;
		// 球に近い場所を求める
		auto nearPos = GetNearestPtOnBox(spherePos, boxPos, BoxB->size, BoxB->rotation);
		// 最近接点と球の中心との長さで判定
		auto nearToSphere = spherePos - nearPos;
		if (nearToSphere.SqLength() < radius * radius)
		{
			info.isHit = true;
			info.hitPos = nearPos;
		}
	}
	if (kindA == ColliderBase::Kind::Box && kindB == ColliderBase::Kind::Sphere)
	{
		auto BoxA = dynamic_cast<ColliderBox*>(colliderA.get());
		auto SphareB = dynamic_cast<ColliderSphere*>(colliderB.get());

		auto spherePos = rigidB->GetPos() + colliderB->GetShift();
		auto boxPos = rigidA->GetPos() + colliderA->GetShift();

		// ボックスの中心から円の中心までのベクトルを作成
		auto boxToSphere = spherePos - boxPos;
		auto radius = SphareB->radius;
		// 球に近い場所を求める
		auto nearPos = GetNearestPtOnBox(spherePos, boxPos, BoxA->size, BoxA->rotation);
		// 最近接点と球の中心との長さで判定
		auto nearToSphere = spherePos - nearPos;
		if (nearToSphere.SqLength() < radius * radius)
		{
			info.isHit = true;
			info.hitPos = nearPos;
		}
	}
	return info;
}

void MyEngine::Physics::FixNextPos(const std::shared_ptr<Rigidbody> primaryRigid, std::shared_ptr<Rigidbody> secondaryRigid, const std::shared_ptr<ColliderBase>& primaryCollider, const std::shared_ptr<ColliderBase>& secondaryCollider, const CollideHitInfo info)
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
			auto primaryToSecondary = (secondaryRigid->GetNextPos() + secondaryCollider->GetShift()) -
				(primaryRigid->GetNextPos() + primaryCollider->GetShift());
			// そのままだとちょうど当たる位置になるので少し余分に離す
			float  awayDist = primarySphere->radius + secondarySphere->radius + 0.0001f;
			// 長さを調整
			primaryToSecondary = primaryToSecondary.GetNormalized() * awayDist;
			// primaryからベクトル方向に押す
			fixedPos = (primaryRigid->GetNextPos() + primaryCollider->GetShift()) + primaryToSecondary;
		}
		if (secondaryKind == ColliderBase::Kind::Box)
		{
			auto dir = (primaryRigid->GetPos() + primaryCollider->GetShift()) - info.hitPos;
			dir.Normalize();
			auto sphereCol = dynamic_cast<ColliderSphere*>(primaryCollider.get());
			DrawSphere3D(info.hitPos.VGet(), 6, 8, 0xffffff, 0xffffff, false);
			fixedPos = info.hitPos + dir * (sphereCol->radius + 0.0001f);
		}
	}
	if (primaryKind == ColliderBase::Kind::Box)
	{
		if (secondaryKind == ColliderBase::Kind::Sphere)
		{
			auto dir = (secondaryRigid->GetPos() + secondaryCollider->GetShift()) - info.hitPos;
			dir.Normalize();
			auto sphereCol = dynamic_cast<ColliderSphere*>(secondaryCollider.get());
			DrawSphere3D(info.hitPos.VGet(), 6, 8, 0xffffff, 0xffffff, false);
			fixedPos = info.hitPos + dir * (sphereCol->radius + 0.0001f);
		}
	}
	secondaryRigid->SetNextPos(fixedPos);
}

void MyEngine::Physics::AddNewCollideInfo(std::shared_ptr<ColideInfo> objA, std::shared_ptr<ColideInfo> objB, SendCollideInfo& info)
{
	std::shared_ptr<ColideInfo> parent = objA;
	std::shared_ptr<ColideInfo> child = objB;
	// Aが親として取得しているか
	bool isParentA = false;
	for (auto& item : info)if (item.first->col == objA->col&&item.first->tag==objA->tag) { parent = item.first; isParentA = true; }
	// Bが親として取得しているか
	bool isParentB = false;
	for (auto& item : info)if (item.first->col == objB->col && item.first->tag == objB->tag){child = item.first; isParentB = true;}
	// AがBどちらかが取得している場合
	if (isParentA || isParentB)
	{
		if (isParentB)
		{
			auto temp = parent;
			parent = child;
			child = temp;
		}
		std::list<std::shared_ptr<ColideInfo>> colideInfo;
		for (auto& item : info)if (item.first->col == parent->col&& item.first->tag == parent->tag)colideInfo = item.second;
		// 親が子を持っているか
		bool isChild = false;
		for (auto& item : colideInfo)if (item->col == child->col&& item->tag == child->tag)isChild = true;
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
		info[parent].emplace_back(child);
	}
}

void MyEngine::Physics::CheckSendOnCollideInfo(SendCollideInfo& preSendInfo, SendCollideInfo& newSendInfo, bool isTrigger)
{
	for (auto& parent : newSendInfo)
	{
		// 以前の情報に親として登録されているか
		bool isPreParent = false;// = preSendInfo.find(parent.first) != preSendInfo.end();
		std::list<std::shared_ptr<MyEngine::Physics::ColideInfo>> preSecond;
		std::shared_ptr<MyEngine::Physics::ColideInfo> preFirst;
		//std::pair<const std::shared_ptr<MyEngine::Physics::ColideInfo>, std::list<std::shared_ptr<MyEngine::Physics::ColideInfo>>> info;
		for (auto& item : preSendInfo)if (item.first->col == parent.first->col){preFirst = item.first; preSecond = item.second; isPreParent = true;}
		bool isAllEnter = true;

		for (auto& child : parent.second)
		{
			std::shared_ptr<MyEngine::Physics::ColideInfo> childFirst;
			bool isPreChild = false;
			if (isPreParent)
			{
				for (auto& item : preSecond)if (item->col == child->col) { childFirst = item; isPreChild = true; }
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
				preSendInfo[preFirst].remove(childFirst);
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
			preSendInfo.erase(preFirst);
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

void MyEngine::Physics::AddOnCollideInfo(std::shared_ptr<ColideInfo> own, std::shared_ptr<ColideInfo> send, OnCollideInfoKind kind)
{
	OnCollideInfoData info;
	info.own = own;
	info.send = send;
	info.kind = kind;
	m_onCollideInfo.emplace_back(info);
}

void MyEngine::Physics::OnCollideInfo(std::shared_ptr<ColideInfo> own, std::shared_ptr<ColideInfo> send, OnCollideInfoKind kind)
{
	auto item = send;
	auto ownCol = own->col;
	auto ownTag = own->tag;

	auto itemCol = item->col;
	auto itemTag = item->tag;

	if (kind == OnCollideInfoKind::CollideEnter)
	{
		ownCol->OnCollideEnter(itemCol,ownTag, itemTag);
	}
	else if (kind == OnCollideInfoKind::CollideStay)
	{
		ownCol->OnCollideStay(itemCol, ownTag, itemTag);
	}
	else if (kind == OnCollideInfoKind::CollideExit)
	{
		ownCol->OnCollideExit(itemCol, ownTag, itemTag);
	}
	else if (kind == OnCollideInfoKind::TriggerEnter)
	{
		ownCol->OnTriggerEnter(itemCol, ownTag, itemTag);
	}
	else if (kind == OnCollideInfoKind::TriggerStay)
	{
		ownCol->OnTriggerStay(itemCol, ownTag, itemTag);
	}
	else if (kind == OnCollideInfoKind::TriggerExit)
	{
		ownCol->OnTriggerExit(itemCol, ownTag, itemTag);
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

Vec3 closestPointOnCube(const Vec3& cubeCenter, const Vec3& size, const Vec3& point) {
	Vec3 closest;

	//クランプ...範囲内に収める(a,min,max)
	closest.x = std::clamp(point.x, cubeCenter.x - size.x, cubeCenter.x + size.x);
	closest.y = std::clamp(point.y, cubeCenter.y - size.y, cubeCenter.y + size.y);
	closest.z = std::clamp(point.z, cubeCenter.z - size.z, cubeCenter.z + size.z);

	return closest;
}

Vec3 closestPointOnCubeAndSphere(const Vec3& cubeCenter, Vec3 size, const Vec3& sphereCenter, double sphereRadius) {
	// 立方体の最近接点
	Vec3 closestPoint = closestPointOnCube(cubeCenter, size, sphereCenter);

	// 球の中心から最近接点までのベクトル
	Vec3 direction = closestPoint - sphereCenter;

	// ベクトルの長さ
	double length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

	// ベクトルを正規化
	if (length > 0) {
		direction = direction * (sphereRadius / length);
	}

	// 最近接点を計算
	return sphereCenter + direction;
}
