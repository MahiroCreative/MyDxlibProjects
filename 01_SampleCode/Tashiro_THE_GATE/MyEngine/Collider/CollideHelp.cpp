#include "CollideHelp.h"
#include <cassert>
#include "Game.h"
#include "Geometry/MathHelp.h"
#include "Rigidbody.h"
#include "Collidable.h"
#include "Colliders.h"
#include "DebugDraw.h"

using namespace MyEngine;

CollideHitInfo IsCollideSphereAndSphere(const Vec3& pt1, const Vec3& pt2, float radius1, float radius2);
CollideHitInfo IsCollideSphereAndCapsule(const Vec3& pt, const Vec3& center, const Vec3& dir, float radius1, float radius2);
CollideHitInfo IsSphereHitSurface(const Vec3& point, float radius, const Vec3& center, const Vec3& norm, const Quaternion& rot, const Vec2& size);
CollideHitInfo IsSphereCheckNormAndSurface(const Vec3& point, float radius, const Vec3& center, const Vec3& norm, const Quaternion& rot, const Vec2& size);
Vec3 FixSurfaceToSphere(const Vec3& surfaceP, const Vec3& norm, const Vec3& sphereP, float radius, const Vec3& sphereShift);
bool IsPreParallelAndNowNotParallel(const Vec3& axis, const Vec3& pre, const Vec3& now);

CollideHitInfo MyEngine::IsCollideSphereAndSphere(const Vec3& posA, const Vec3& posB, ColliderSphere* colliderA, ColliderSphere* colliderB)
{
    auto sphereAPos = posA + colliderA->center;
    auto sphereBPos = posB + colliderB->center;

    return IsCollideSphereAndSphere(sphereAPos, sphereBPos, colliderA->radius, colliderB->radius);
}

CollideHitInfo MyEngine::IsCollideSphereAndCapsule(const Vec3& posSphere, const Vec3& posCapsule, ColliderSphere* colliderSphere, ColliderCapsule* colliderCapsule)
{
    auto spherePos = posSphere + colliderSphere->center;
    auto capsulePos = posCapsule + colliderCapsule->center;

    auto halfDir = colliderCapsule->dir * colliderCapsule->size * 0.5f;

    return IsCollideSphereAndCapsule(spherePos, capsulePos, halfDir, colliderCapsule->radius, colliderSphere->radius);
}

CollideHitInfo MyEngine::IsCollideSphereAndTerrain(const Vec3& posSphere, const Vec3& posTerrain, ColliderSphere* colliderSphere, ColliderTerrain* colliderTerrain)
{
    auto spherePos  = posSphere + colliderSphere->center;
    auto terrainPos = posTerrain + colliderTerrain->center;

    return IsSphereHitSurface(spherePos, colliderSphere->radius, terrainPos, colliderTerrain->norm, colliderTerrain->rotation, colliderTerrain->size);
}

CollideHitInfo MyEngine::IsCollideSphereAndBox(const Vec3& posSphere, const Vec3& posBox, ColliderSphere* colliderSphere, ColliderBox* colliderBox, const PreHitInfo& preInfo)
{
    auto spherePos = posSphere + colliderSphere->center;
    auto boxPos = posBox + colliderBox->center;

    // 球に近い場所を求める
    auto nearPos = GetNearestPtOnBox(spherePos, boxPos, colliderBox->size, colliderBox->rotation);
    
    // 最近接点と球の中心との長さで判定
    auto radius = colliderSphere->radius;
    auto nearToSphere = spherePos - nearPos;
    if (nearToSphere.SqLength() < radius * radius)
    {
        CollideHitInfo info;
        info.isHit = true;
        info.hitPos = nearPos;
        info.fixDir = nearToSphere.GetNormalized();

        return info;
    }
    else
    {
        return CollideHitInfo{ false };
    }
}

CollideHitInfo MyEngine::IsCollideCapsuleAndBox(const Vec3& posCapsule, const Vec3& posBox, ColliderCapsule* colliderCapsule, ColliderBox* colliderBox, const PreHitInfo& preInfo)
{
    auto capsulePos = posCapsule + colliderCapsule->center;
    auto boxPos = posBox + colliderBox->center;

    auto dirH = colliderCapsule->dir * colliderCapsule->size * 0.5f;
    auto cS = capsulePos + dirH;
    auto cE = capsulePos - dirH;

    auto cNearPos = GetNearestPtOnLine(boxPos, cS, cE);
    auto bNearPos = GetNearestPtOnBox(cNearPos, boxPos, colliderBox->size, colliderBox->rotation);

    // 最近接点と球の中心との長さで判定
    auto radius = colliderCapsule->radius;
    auto bNearToCNear = cNearPos - bNearPos;
    if (bNearToCNear.SqLength() > radius * radius) return CollideHitInfo{ false };

    CollideHitInfo info;
    info.isHit = true;
    info.hitPos = bNearPos;
    info.fixDir = cNearPos - bNearPos;
    info.fixDir.Normalize();

    if (preInfo.col)
    {
        bool isCheck = true;
        if (isCheck)
        {
            if (IsPreParallelAndNowNotParallel(Vec3::Right(), preInfo.info.fixDir, info.fixDir)) return CollideHitInfo{ false };
            if (IsPreParallelAndNowNotParallel(Vec3::Up(),    preInfo.info.fixDir, info.fixDir)) return CollideHitInfo{ false };
            if (IsPreParallelAndNowNotParallel(Vec3::Front(), preInfo.info.fixDir, info.fixDir)) return CollideHitInfo{ false };
        }
    }

    return info;
}

CollideHitInfo MyEngine::IsCollideCapsuleAndCapsule(const Vec3& posA, const Vec3& posB, ColliderCapsule* colliderA, ColliderCapsule* colliderB)
{
    auto capsuleAPos = posA + colliderA->center;
    auto capsuleBPos = posB + colliderB->center;

    // 自身の向いている方向に伸びているベクトルを作成
    Vec3 dirA = colliderA->dir.GetNormalized() * colliderA->size * 0.5f;
    // 対象の向いている方向に伸びているベクトルを作成
    Vec3 dirB = colliderB->dir.GetNormalized() * colliderB->size * 0.5f;
    // 結果保持
    Vec3 minPosA, minPosB;
    GetNearestPtOnLine(capsuleAPos, dirA, capsuleBPos, dirB, minPosA, minPosB);

    return IsCollideSphereAndSphere(minPosA, minPosB, colliderA->radius, colliderB->radius);
}

CollideHitInfo MyEngine::IsCollideCapsuleAndTerrain(const Vec3& posCapsule, const Vec3& posTerrain, ColliderCapsule* colliderCapsule, ColliderTerrain* colliderTerrain)
{
    assert(false && "修正中の判定です");
    return CollideHitInfo{ false };
#if false
    auto capsulePos = posCapsule + colliderCapsule->center;
    auto terrainPos = posTerrain + colliderTerrain->center;

    auto halfSize = colliderCapsule->size * 0.5f;
    auto capsuleStart = capsulePos + colliderCapsule->dir * halfSize;
    auto capsuleEnd = capsulePos - colliderCapsule->dir * halfSize;

    auto centerToStart = capsuleStart - terrainPos;
    auto centerToEnd = capsuleEnd - terrainPos;

    // 面をまたいでいる場合
    if (Dot(centerToEnd, colliderTerrain->norm) * Dot(centerToStart, colliderTerrain->norm) <= 0)
    {
        // 線のスタート・ゴールと面までの長さを取得
        auto startSurfaceLen = Projection(centerToStart, colliderTerrain->norm).Length();
        auto endSurfaceLen = Projection(centerToEnd, colliderTerrain->norm).Length();
        // 交点が線分上の何処あたりかを計算
        float rate = startSurfaceLen / (startSurfaceLen + endSurfaceLen);

        // 交点を取得
        auto crossPt = terrainPos + centerToStart * (1 - rate) + centerToEnd * rate;

        auto info = IsSphereHitSurface(crossPt, colliderCapsule->radius, terrainPos, colliderTerrain->norm, colliderTerrain->rotation, colliderTerrain->size);
        if (info.isHit)
        {
            // どちらよりかをチェック
            if (rate < 0.5f)
            {
                colliderCapsule->SetFixS(true);
                colliderCapsule->SetFixE(false);
            }
            else
            {
                colliderCapsule->SetFixS(false);
                colliderCapsule->SetFixE(true);
            }
        }
        return info;
    }
    // 面をまたいでいない場合
    else
    {
        // 先端、終端の円で判定をとる
        auto infoStart = IsSphereHitSurface(capsuleStart, colliderCapsule->radius, terrainPos, colliderTerrain->norm, colliderTerrain->rotation, colliderTerrain->size);
        auto infoEnd = IsSphereHitSurface(capsuleEnd, colliderCapsule->radius, terrainPos, colliderTerrain->norm, colliderTerrain->rotation, colliderTerrain->size);
        colliderCapsule->SetFixS(infoStart.isHit);
        colliderCapsule->SetFixE(infoEnd.isHit);
        // 両方とも当たっている場合
        if (infoStart.isHit && infoEnd.isHit)
        {
            // 当たった場所を中心とする
            auto pos = (infoStart.hitPos + infoEnd.hitPos) * 0.5f;
            return CollideHitInfo{ true, pos };
        }
        // 片方のみ当たっている場合
        else if (infoStart.isHit)
        {
            return infoStart;
        }
        else
        {
            return infoEnd;
        }
    }
#endif;
}

CollideHitInfo MyEngine::IsCollideLineAndTerrain(const Vec3& posLine, const Vec3& posTerrain, ColliderLine* colliderLine, ColliderTerrain* colliderTerrain)
{
    const auto& start = posLine + colliderLine->center;
    const auto& end = start + colliderLine->dir * colliderLine->len;
    const auto& terrainPos = posTerrain + colliderTerrain->center;

    const auto& vec1 = start - terrainPos;
    const auto& vec2 = end - terrainPos;

    // 線が面またいでいなければ終了
    if (Vec3::Dot(vec1, colliderTerrain->norm) * Vec3::Dot(vec2, colliderTerrain->norm) > 0) return CollideHitInfo{ false };

    // 線と面の交点を求める
    auto crossPt = GetCrossPtSurfaceAndLine(start, end, terrainPos, colliderTerrain->norm);

    // 左右方向・前後方向の長さを確認して大きさ以内なら当たっている
    auto right = colliderTerrain->rotation * Vec3::Right();
    auto front = colliderTerrain->rotation * Vec3::Front();
    auto halfSize = colliderTerrain->size * 0.5f;
    auto centerToCross = crossPt - terrainPos;
    auto rp = Vec3::Projection(centerToCross, right);
    auto fp = Vec3::Projection(centerToCross, front);

    if (rp.SqLength() > (halfSize.x * halfSize.x)) return CollideHitInfo{ false };
    if (fp.SqLength() > (halfSize.y * halfSize.y)) return CollideHitInfo{ false };

    return CollideHitInfo{ true, crossPt };
}

Vec3 MyEngine::FixSphereToSphere(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderSphere* primaryCollider, ColliderSphere* secondaryCollider, const CollideHitInfo& hitInfo)
{
    auto priPos = primaryPos + primaryCollider->center;
    auto secPos = secondaryPos + secondaryCollider->center;

    // primaryからsecondaryへのベクトルを作成
    auto primaryToSecondary = secPos - priPos;
    // そのままだとちょうど当たる位置になるので少し余分に離す
    float  awayDist = primaryCollider->radius + secondaryCollider->radius + 0.0001f;
    // 長さを調整
    primaryToSecondary = primaryToSecondary.GetNormalized() * awayDist;

    // primaryからベクトル方向に押す
    return priPos + primaryToSecondary - secondaryCollider->center;
}

Vec3 MyEngine::FixCapsuleToCapsule(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderCapsule* primaryCollider, ColliderCapsule* secondaryCollider, const CollideHitInfo& hitInfo)
{
    auto priPos = primaryPos + primaryCollider->center;
    auto secPos = secondaryPos + secondaryCollider->center;

    // 自身の向いている方向に伸びているベクトルを作成
    Vec3 dirPrimary = primaryCollider->dir.GetNormalized() * primaryCollider->size * 0.5f;
    // 対象の向いている方向に伸びているベクトルを作成
    Vec3 dirSecondary = secondaryCollider->dir.GetNormalized() * secondaryCollider->size * 0.5f;
    // 結果保持
    Vec3 minPosPrimary, minPosSecondary;
    // 線分同士の最近接点を取得する
    GetNearestPtOnLine(priPos, dirPrimary, secPos, dirSecondary, minPosPrimary, minPosSecondary);

    // Primaryの最近接点からSecondaryの最近接点までのベクトル
    auto pToS = minPosSecondary - minPosPrimary;
    // 長さを取得
    auto len = pToS.Length();
    // それぞれの半径＋極小値から長さを引いたぶんだけずらす
    float awayDis = primaryCollider->radius + secondaryCollider->radius + 0.0001f - len;

    return secPos - secondaryCollider->center + pToS.GetNormalized() * awayDis;
}

Vec3 MyEngine::FixCapsuleToSphere(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderCapsule* primaryCollider, ColliderSphere* secondaryCollider, const CollideHitInfo& hitInfo)
{
    auto capsulePos = primaryPos + primaryCollider->center;
    auto spherePos = secondaryPos + secondaryCollider->center;

    auto dirH = primaryCollider->dir * primaryCollider->size * 0.5f;
    auto cS = capsulePos + dirH;
    auto cE = capsulePos - dirH;

    auto cNearPos = GetNearestPtOnLine(spherePos, cS, cE);

    auto dir = spherePos - cNearPos;
    dir.Normalize();

    return cNearPos + dir * (primaryCollider->radius + secondaryCollider->radius + 0.0001f) - secondaryCollider->center;
}

Vec3 MyEngine::FixTerrainToSphere(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderTerrain* primaryCollider, ColliderSphere* secondaryCollider, const CollideHitInfo& hitInfo)
{
    auto terrainPos = primaryPos + primaryCollider->center;
    auto spherePos = secondaryPos + secondaryCollider->center;

    return FixSurfaceToSphere(terrainPos, primaryCollider->norm, spherePos, secondaryCollider->radius, secondaryCollider->center);
}

Vec3 MyEngine::FixTerrainToCapsule(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderTerrain* primaryCollider, ColliderCapsule* secondaryCollider, const CollideHitInfo& hitInfo)
{
    // FIXME: 修正
    assert(false && "修正中の関数です");
    return secondaryPos;
#if false
    auto terrainPos = primaryPos + primaryCollider->center;
    auto capsulePos = secondaryPos + secondaryCollider->center;

    auto halfSize = secondaryCollider->size * 0.5f;
    auto halfSizeDir = secondaryCollider->dir * halfSize;
    auto capsuleStart = capsulePos + halfSizeDir;
    auto capsuleEnd = capsulePos - halfSizeDir;

    bool isFixS = secondaryCollider->IsFixS();
    bool isFixE = secondaryCollider->IsFixE();

    if (isFixS)
    {
        capsuleStart = FixSurfaceToSphere(terrainPos, primaryCollider->norm, capsuleStart, secondaryCollider->radius, secondaryCollider->center);
    }
    else
    {
        capsuleStart -= secondaryCollider->center;
    }
    if (isFixE)
    {
        capsuleEnd = FixSurfaceToSphere(terrainPos, primaryCollider->norm, capsuleEnd, secondaryCollider->radius, secondaryCollider->center);
    }
    else
    {
        capsuleEnd -= secondaryCollider->center;
    }

    auto dir = (capsuleStart - capsuleEnd).GetNormalized();
    dir *= halfSize;

    Vec3 res;
    if (IsUniformDirection(primaryCollider->norm, secondaryCollider->dir))
    {
        res = capsuleEnd + dir;
    }
    else
    {
        res = capsuleStart - dir;
    }
    // 法線方向に少しずらす
    res += primaryCollider->norm * 0.001f;

    
    return res;
#endif
}

Vec3 MyEngine::FixBoxToSphere(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderBox* primaryCollider, ColliderSphere* secondaryCollider, const CollideHitInfo& hitInfo)
{
    return hitInfo.hitPos + hitInfo.fixDir * (secondaryCollider->radius + 0.0001f);
}

Vec3 MyEngine::FixBoxToCapsule(const Vec3& primaryPos, const Vec3& secondaryPos, ColliderBox* primaryCollider, ColliderCapsule* secondaryCollider, const CollideHitInfo& hitInfo)
{
    auto capsulePos = secondaryPos + secondaryCollider->center;

    auto dirH = secondaryCollider->dir * secondaryCollider->size * 0.5f;
    auto cS = capsulePos + dirH;
    auto cE = capsulePos - dirH;

    auto cNearPos = GetNearestPtOnLine(hitInfo.hitPos, cS, cE);

    auto dir = cNearPos - hitInfo.hitPos;
    auto fixLen = secondaryCollider->radius - dir.Length();
    
    return capsulePos + hitInfo.fixDir * (fixLen + 0.0001f) - secondaryCollider->center;
}

CollideHitInfo IsCollideSphereAndSphere(const Vec3& pt1, const Vec3& pt2, float radius1, float radius2)
{
    auto pt1Topt2 = (pt2 - pt1);
    auto sqLen = pt1Topt2.SqLength();
    auto sumRadius = radius1 + radius2;
    CollideHitInfo info;
    if (sqLen < sumRadius * sumRadius)
    {
        info.isHit = true;
        float len = pt1Topt2.Length();
        info.hitPos = pt1 + pt1Topt2.GetNormalized() * len * (radius1 / sumRadius);
    }
    return info;
}

CollideHitInfo IsCollideSphereAndCapsule(const Vec3& pt, const Vec3& center, const Vec3& dir, float radius1, float radius2)
{
    auto capsuleStart = center + dir;
    auto capsuleEnd = center - dir;

    auto nearPt = GetNearestPtOnLine(pt, capsuleStart, capsuleEnd);

    return IsCollideSphereAndSphere(pt, nearPt, radius1, radius2);
}

CollideHitInfo IsSphereHitSurface(const Vec3& point, float radius, const Vec3& center, const Vec3& norm, const Quaternion& rot, const Vec2& size)
{
    // 円の中心から面までの線を作成
    auto ptToCenter = -norm * radius;
    auto ptToCenterPos = point + ptToCenter;

    // 線と面の交点を求める
    auto crossPt = GetCrossPtSurfaceAndLine(point, ptToCenterPos, center, norm);

    // 左右方向・前後方向の長さを確認して大きさ以内なら当たっている
    auto right = rot * Vec3::Right();
    auto front = rot * Vec3::Front();
    auto centerToCross = crossPt - center;
    auto halfSize = size * 0.5f;
    auto rp = Vec3::Projection(centerToCross, right);
    auto fp = Vec3::Projection(centerToCross, front);

    // 長さの修正
    if (rp.SqLength() > (halfSize.x) * (halfSize.x))
    {
        rp = rp.GetNormalized() * halfSize.x;
    }
    if (fp.SqLength() > (halfSize.y) * (halfSize.y))
    {
        fp = fp.GetNormalized() * halfSize.y;
    }

    crossPt = rp + fp + center;

    CollideHitInfo info;
    if ((crossPt - point).SqLength() < radius * radius)
    {
        info.isHit = true;
        info.hitPos = crossPt;
    }
    return info;
}

CollideHitInfo IsSphereCheckNormAndSurface(const Vec3& point, float radius, const Vec3& center, const Vec3& norm, const Quaternion& rot, const Vec2& size)
{
    auto pt = GetCrossPtSurfaceAndPt(point, center, norm);
    auto v = pt - center;
    // ある程度同じ方向
    if (Vec3::IsUniformDirection(norm, v))
    {
        return IsSphereHitSurface(point, radius, center, norm, rot, size);
    }
    else
    {
        return CollideHitInfo{ false };
    }
}

Vec3 FixSurfaceToSphere(const Vec3& surfaceP, const Vec3& norm, const Vec3& sphereP, float radius, const Vec3& sphereShift)
{
    // 球の中心の面での位置を取得
    auto crossPt = GetCrossPtSurfaceAndPt(sphereP, surfaceP, norm);
    crossPt -= sphereShift;

    // 地面の法線方向に少し余分にずらす
    float awayDis = radius + 0.001f;
    auto normDisVec = norm.GetNormalized() * awayDis;
    return crossPt + normDisVec;
}

bool IsPreParallelAndNowNotParallel(const Vec3& axis, const Vec3& pre, const Vec3& now)
{
    float dot = Vec3::Dot(pre, axis);
    // 一つ前が平行でないならfalse
    if (-1.0f < dot && dot < 1.0f) return false;

    dot = Vec3::Dot(now, axis);
    // 今回が平行ならばfalse、平行でなければtrue
    return -1.0f < dot && dot < 1.0f;
}