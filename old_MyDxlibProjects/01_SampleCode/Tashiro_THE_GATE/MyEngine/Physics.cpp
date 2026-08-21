#include "Physics.h"
#include <cassert>
#include "Game.h"
#include "Collidable.h"
#include "Collider/Colliders.h"
#include "DebugDraw.h"
#include "SceneManager.h"

using namespace MyEngine;

namespace
{
    // 判定回数
    constexpr int CHECK_MAX_COUNT = 1000;
    // 分割判定速度
//    constexpr float CHECK_DIVISION_SPEED = 0.2f;
    constexpr float CHECK_DIVISION_SPEED = 0.7f;
    // 判定するオブジェクト同士の距離
    constexpr float CHECK_COLLIDE_LENDGHT = 22.0f;
    constexpr float CHECK_COLLIDE_SQ_LENDGHT = CHECK_COLLIDE_LENDGHT * CHECK_COLLIDE_LENDGHT;
}

Physics::Physics() :
    m_objects{},
    m_onCollideInfo{},
    m_newCollideInfo{},
    m_preCollideInfo{},
    m_newTirrigerInfo{},
    m_preTirrigerInfo{}
{
}

Physics& Physics::GetInstance()
{
    static Physics instance;
    return instance;
}

void Physics::Entry(std::shared_ptr<Collidable> collidable)
{
    bool isFound = std::find(m_objects.begin(), m_objects.end(), collidable) != m_objects.end();
    // 未登録なら追加
    if (!isFound)
    {
        m_objects.emplace_back(collidable);
    }
    // 登録済みなら無視
    else
    {
        assert(false && "既にエントリーしています");
    }
}

void Physics::Exit(std::shared_ptr<Collidable> collidable)
{
    auto it = std::find(m_objects.begin(), m_objects.end(), collidable);
    // 登録済みなら削除
    if (it != m_objects.end())
    {
        m_objects.erase(it);
    }
    // 未登録なら無視
    else
    {
        assert(false && "エントリーしていません");
    }
}

void Physics::Update()
{
    // オプションorポーズが開いていれば更新しない
    auto& scnMgr = SceneManager::GetInstance();
    if (scnMgr.IsOpenOption()) return;

    // 通知リストのクリア・更新
    m_onCollideInfo.clear();
    m_preCollideInfo = m_newCollideInfo;
    m_newCollideInfo.clear();
    m_preTirrigerInfo = m_newTirrigerInfo;
    m_newTirrigerInfo.clear();

    // 次の移動先を仮決定
    MoveNextPos();
    // 判定確認
    CheckCollide();
    // 通知リストを確認
    CheckSendOnCollideInfo(m_preCollideInfo, m_newCollideInfo, false);
    CheckSendOnCollideInfo(m_preTirrigerInfo, m_newTirrigerInfo, true);
    // 座標を決定
    FixPos();
    // 通知を送る
    for (const auto& item : m_onCollideInfo)
    {
        OnCollideInfo(item.own, item.send, item.ownColIndex, item.sendColIndex, item.hitInfo, item.kind);
    }
}

std::vector<Physics::HitTagObjectInfo> Physics::GetHitObject(const Vec3& center, ColliderBase* collider, ObjectTag tag, const PreHitInfo& preInfo, bool isOne, Collidable* through)
{
    std::vector<HitTagObjectInfo> res;

    for (auto& item : m_objects)
    {
        // 違うタグなら次のオブジェクトへ
        if (item->GetTag() != tag) continue;
        // スルーするオブジェクトなら次へ
        if (item.get() == through) continue;

        // 余りにも離れすぎていたら判定しない
        if ((center - item->GetPos()).SqLength() > CHECK_COLLIDE_SQ_LENDGHT) continue;

        // 判定全て回す
        for (auto& col : item->m_collider)
        {
            auto info = collider->IsCollide(center, col.get(), item->GetPos(), preInfo);
            // もしtrueなら追加して次の判定へ
            if (info.isHit)
            {
                res.push_back(HitTagObjectInfo{ item.get(), info });
                // 一つでOKならこれで終了
                if (isOne) return res;
                break;
            }
        }
    }

    return res;
}

void Physics::MoveNextPos() const
{
    int k = 0;
    for (const auto& item : m_objects)
    {
        auto& rigid = item->m_rigid;

        // 重力処理
        if (rigid.IsGravity())
        {
            auto vel = rigid.GetVelocity();
            vel.y -= Game::GRAVITY;
            if (vel.y < -Game::GRAVITY_MAX_SPEED)
            {
                vel.y = -Game::GRAVITY_MAX_SPEED;
            }
            rigid.SetVelocity(vel);
        }

        // 座標に移動量を追加
        auto pos = rigid.GetPos();
        auto nextPos = pos + rigid.GetVelocity();
        rigid.SetNextPos(nextPos);

#ifdef _DEBUG
        // デバック情報
        AddDebugInfo(pos, item->m_collider, DebugDraw::COL_BEFOR);
        AddDebugInfo(nextPos, item->m_collider, DebugDraw::COL_NEXT);
#endif
    }
}

void Physics::CheckCollide()
{
    // 判定リスト取得
    const auto& colVec = GetCollisionList();
    // 判定リストはペアになっているので半分の数だけ繰り返す
    int colNum = static_cast<int>(colVec.size() * 0.5f);
    // 判定回数
    int count = 0;

    std::unordered_map<std::shared_ptr<Collidable>, std::unordered_map<ObjectTag, PreHitInfo>> preHitInfoList;
    while (true)
    {
        preHitInfoList.clear();
        bool isNoHit = true;
        for (int i = 0; i < colNum; ++i)
        {
            auto objA = colVec[i * 2];
            auto objB = colVec[i * 2 + 1];

            // それぞれが持つ判定全てを比較
            for (int i = 0; i < objA->m_collider.size(); ++i)
            {
                for (int j = 0; j < objB->m_collider.size(); ++j)
                {
                    auto& colA = objA->m_collider.at(i);
                    auto& colB = objB->m_collider.at(j);

                    // 優先度に合わせて変数を変更
                    auto primary = objA;
                    auto secondary = objB;
                    auto primaryCollider = colA;
                    auto secondaryCollider = colB;
                    auto index = i;
                    // Aの方が優先度低い場合
                    if (objA->m_priority < objB->m_priority)
                    {
                        primary = objB;
                        secondary = objA;
                        primaryCollider = colB;
                        secondaryCollider = colA;
                        index = j;
                    }
                    // 優先度が変わらない場合
                    else if (objA->m_priority == objB->m_priority)
                    {
                        // 速度の速いを方を優先度が高いことにする
                        if (objA->m_rigid.GetVelocity().SqLength() < objB->m_rigid.GetVelocity().SqLength())
                        {
                            primary = objB;
                            secondary = objA;
                            primaryCollider = colB;
                            secondaryCollider = colA;
                            index = j;
                        }
                    }

                    CollideHitInfo collideHitInfo;
                    auto& preHitInfo = preHitInfoList[secondary][primary->GetTag()];
                    auto speed = secondary->GetVelocity().Length();
                    Vec3 checkPos = secondary->m_rigid.GetNextPos();
                    // 分割する速度に達していなければ純粋に分割せずに判定
                    if (speed < CHECK_DIVISION_SPEED)
                    {
                        collideHitInfo = secondaryCollider->IsCollide(checkPos, primaryCollider.get(), primary->m_rigid.GetNextPos(), preHitInfo);
                    }
                    // 達していれば座標を分割して判定
                    else
                    {
                        int num = static_cast<int>(speed / CHECK_DIVISION_SPEED) + 1;
                        auto divisionDir = secondary->m_rigid.GetNextPos() - secondary->GetPos();
                        float y = divisionDir.y;
                        divisionDir.y = 0;
                        divisionDir = divisionDir / static_cast<float>(num);
                        checkPos = secondary->GetPos() + divisionDir;
                        checkPos.y += y;
                        for (int k = 0; k < num; ++k)
                        {
#ifdef _DEBUG
                            AddDebugInfo(checkPos, secondary->m_collider, DebugDraw::COL_HIT);
#endif
                            collideHitInfo = secondaryCollider->IsCollide(checkPos, primaryCollider.get(), primary->m_rigid.GetNextPos(), preHitInfo);
                            if (collideHitInfo.isHit) break;
                            checkPos += divisionDir;
                        }
                    }
                    // 当たっていなければ次の判定に
                    if (!collideHitInfo.isHit) continue;

                    if (objA->GetTag() == ObjectTag::PALYER || objB->GetTag() == ObjectTag::PALYER)
                    {
                        int A = 0;
                    }

                    // 通過オブジェクト確認
                    auto throughA = objA->m_throughTag;
                    auto throughB = objB->m_throughTag;
                    bool isThrough = std::find(throughA.begin(), throughA.end(), objB->GetTag()) != throughA.end()
                        || std::find(throughB.begin(), throughB.end(), objA->GetTag()) != throughB.end();
                    // isTriggerがtrueか通過オブジェクトなら通知だけ追加して次の判定に
                    bool isTrigger = colA->isTrigger || colB->isTrigger || isThrough;
                    if (isTrigger)
                    {
                        AddNewCollideInfo(objA, objB, i, j, m_newTirrigerInfo, collideHitInfo);
                        continue;
                    }

                    // 通知を追加
                    AddNewCollideInfo(objA, objB, i, j, m_newCollideInfo, collideHitInfo);

                    // 次の座標を修正する
                    if (primary->GetTag() == ObjectTag::FLOOR)
                    {
                        const auto& box = std::dynamic_pointer_cast<ColliderBox>(primaryCollider);
                        collideHitInfo.fixDir = box->norm;
                    }
                    auto nextPos = primaryCollider->FixNextPos(primary->m_rigid.GetNextPos(), secondaryCollider.get(), checkPos, collideHitInfo);
                    secondary->m_rigid.SetNextPos(nextPos);

                    preHitInfo = { collideHitInfo, primary, index };

                    isNoHit = false;
                }
            }
        }

        // 判定回数増加
        count++;
        // 当たっていなければ終了
        if (isNoHit) break;
        // 最大回数確認したら修正がうまく出来ていなくても終了する
        if (count > CHECK_MAX_COUNT) break;
    }
    
#ifdef _DEBUG
    printf("判定回数:%d\nループ回数:%d\n", colNum, count - 1);
#endif
}

std::vector<std::shared_ptr<Collidable>> Physics::GetCollisionList() const
{
    std::vector<std::shared_ptr<Collidable>> res;

    for (int i = 0; i < m_objects.size(); ++i)
    {
        for (int j = i + 1; j < m_objects.size(); ++j)
        {
            auto& obj1 = m_objects[i];
            auto& obj2 = m_objects[j];

            // 移動しないオブジェクト同士なら判定しない
            if (obj1->GetPriority() == Collidable::Priority::STATIC && obj2->GetPriority() == Collidable::Priority::STATIC) continue;

            // 何かしらの判定の中心にいた場合に追加
            bool isAdd = false;
            for (auto& col1 : obj1->m_collider)
            {
                for (auto& col2 : obj2->m_collider)
                {
                    const auto& pos1 = obj1->GetPos() + col1->center;
                    const auto& pos2 = obj2->GetPos() + col2->center;
                    if ((pos1 - pos2).SqLength() < CHECK_COLLIDE_SQ_LENDGHT)
                    {
                        // 判定リストに追加
                        res.push_back(obj1);
                        res.push_back(obj2);
                        isAdd = true;
                        break;
                    }
                }
                if (isAdd) break;
            }
        }
    }

    return res;
}

void Physics::AddNewCollideInfo(const std::weak_ptr<Collidable>& objA, const std::weak_ptr<Collidable>& objB, int colIndexA, int colIndexB, SendCollideInfo& info, const CollideHitInfo& hitInfo)
{
    // 既に追加されている通知リストにあれば追加しない
    for (auto& i : info)
    {
        if (i.own.lock() == objA.lock() && i.send.lock() == objB.lock() && i.ownColIndex == colIndexA && i.sendColIndex == colIndexB) return;
        if (i.own.lock() == objB.lock() && i.send.lock() == objA.lock() && i.ownColIndex == colIndexB && i.sendColIndex == colIndexA) return;
    }

    // ここまで来たらまだ通知リストに追加されていないため追加
    info.emplace_back(SendInfo{ objA, objB, colIndexA, colIndexB, hitInfo });
}

void Physics::CheckSendOnCollideInfo(SendCollideInfo& preSendInfo, SendCollideInfo& newSendInfo, bool isTrigger)
{
    // 1つ前に通知リストが当たったか
    auto isPreExist = preSendInfo.size() != 0;

    for (auto& info : newSendInfo)
    {
        bool isEntry = false;

        // 1つ前に通知リストがあった場合
        if (isPreExist)
        {
            bool isEnter = true;
            // 1つ前の通知リストをすべて回す
            auto it = preSendInfo.begin();
            for (; it != preSendInfo.end(); ++it)
            {
                // 通知リストが存在した場合は当たった時の通知を呼ばないようにする
                if (it->own.lock() == info.own.lock() && it->send.lock() == info.send.lock() && it->ownColIndex == info.ownColIndex) isEnter = false;
                if (it->own.lock() == info.send.lock() && it->send.lock() == info.own.lock() && it->ownColIndex == info.ownColIndex) isEnter = false;
                if (!isEnter) break;
            }
            if (isEnter)
            {
                // 当たった時の通知を追加
                if (isTrigger) AddOnCollideInfo(info, OnCollideInfoKind::TriggerEnter);
                else           AddOnCollideInfo(info, OnCollideInfoKind::CollideEnter);
            }
            else
            {
                // 一つ前の通知リストから今回存在する通知のものを削除する
                preSendInfo.erase(it);
            }
        }
        // 1つ前に通知リストがなかった場合
        else
        {
            // 当たった時の通知を追加
            if (isTrigger) AddOnCollideInfo(info, OnCollideInfoKind::TriggerEnter);
            else           AddOnCollideInfo(info, OnCollideInfoKind::CollideEnter);
        }

        // 当たっている状態の通知を追加
        if (isTrigger) AddOnCollideInfo(info, OnCollideInfoKind::TriggerStay);
        else           AddOnCollideInfo(info, OnCollideInfoKind::CollideStay);
    }

    // 上で削除されずに残った1つ前の通知リストは今回抜けているため
    // 離れた時の通知を追加
    for (auto& info : preSendInfo)
    {
        if (isTrigger) AddOnCollideInfo(info, OnCollideInfoKind::TriggerExit);
        else           AddOnCollideInfo(info, OnCollideInfoKind::CollideExit);
    }
}

void Physics::AddOnCollideInfo(const SendInfo& info, OnCollideInfoKind kind)
{
    m_onCollideInfo.emplace_back(OnCollideInfoData{info.own, info.send, info.ownColIndex, info.sendColIndex, info.hitInfo, kind});
    m_onCollideInfo.emplace_back(OnCollideInfoData{info.send, info.own, info.sendColIndex, info.ownColIndex, info.hitInfo, kind});
}

void Physics::OnCollideInfo(const std::weak_ptr<Collidable>& own, const std::weak_ptr<Collidable>& send, int ownIndex, int sendIndex, const CollideHitInfo& hitInfo, OnCollideInfoKind kind)
{
    // 送る側、受け取る側どちらかのリンクが途切れていたら通知をやめる
    if (own.expired() || send.expired()) return;

    // 種類に合わせて通知を送る
    if (kind == OnCollideInfoKind::CollideEnter)
    {
        own.lock()->OnCollideEnter(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
    else if (kind == OnCollideInfoKind::CollideStay)
    {
        own.lock()->OnCollideStay(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
    else if (kind == OnCollideInfoKind::CollideExit)
    {
        own.lock()->OnCollideExit(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
    else if (kind == OnCollideInfoKind::TriggerEnter)
    {
        own.lock()->OnTriggerEnter(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
    else if (kind == OnCollideInfoKind::TriggerStay)
    {
        own.lock()->OnTriggerStay(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
    else if (kind == OnCollideInfoKind::TriggerExit)
    {
        own.lock()->OnTriggerExit(send.lock().get(), ownIndex, sendIndex, hitInfo);
    }
}

/// <summary>
/// 最終的な未来の座標から現在の座標に適用
/// </summary>
void Physics::FixPos() const
{
    for (const auto& item : m_objects)
    {
        auto& rigid = item->m_rigid;
        // 座標の確定
        rigid.SetPos(rigid.GetNextPos());
#ifdef _DEBUG
        // デバック情報
        AddDebugInfo(rigid.GetPos(), item->m_collider, DebugDraw::COL_AFFTER);
#endif
    }
}

#ifdef _DEBUG
void MyEngine::Physics::AddDebugInfo(const Vec3& pos, const std::vector<std::shared_ptr<ColliderBase>>& colliders, unsigned int color) const
{
    auto& debug = DebugDraw::GetInstance();

    for (auto& col : colliders)
    {
        auto kind = col->GetKind();
        if (kind == ColKind::SPHERE)
        {
            auto data = dynamic_cast<ColliderSphere*>(col.get());
            debug.DrawSphere(pos + data->center, data->radius, color, false);
        }
        else if (kind == ColKind::CAPSULE)
        {
            auto data = dynamic_cast<ColliderCapsule*>(col.get());
            debug.DrawCapsule(pos + data->center, data->dir, data->radius, data->size, color, false);
        }
        else if (kind == ColKind::TERRAIN)
        {
            auto data = dynamic_cast<ColliderTerrain*>(col.get());
            debug.DrawTerrain(pos + data->center, data->norm, data->size, data->rotation, color, false);
        }
        else if (kind == ColKind::BOX)
        {
            auto data = dynamic_cast<ColliderBox*>(col.get());
            debug.DrawCube(pos + data->center, data->size, data->rotation, color, false);
        }
    }
}
#endif