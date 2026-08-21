#pragma once
#include "Vec3.h"
#include "Quaternion.h"

// FIXME: ここのサマリーを書く

// 線分同士が平行か判定
bool IsParallelOnLine(const Vec3& startA, const Vec3& endA, const Vec3& startB, const Vec3& endB);
// 線分が面をまたいでいるか判定
bool IsCrossSurfaceOnLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm);

// 最近接点を取得
Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end);
Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end, float& rate);
// 線分同士の最近接点を取得
void GetNearestPtOnLine(const Vec3& centerA, const Vec3& dirA, const Vec3& centerB, const Vec3& dirB, Vec3& outPtA, Vec3& outPtB);
// ボックス上での最近接点を取得
Vec3 GetNearestPtOnBox(const Vec3& pos, const Vec3& boxPos, const Vec3& boxScale, const Quaternion& rot);
// 線分上に最近接点があるかの判定
bool IsNearestPointOnLine(const Vec3& point, const Vec3& start, const Vec3& end);


// 線と面の交点を取得
Vec3 GetCrossPtSurfaceAndLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm);
// 点と面の交点を取得
Vec3 GetCrossPtSurfaceAndPt(const Vec3& pt, const Vec3& center, const Vec3& norm);

// 2つの線分の回転クオータオンを取得
Quaternion GetQuaternion(const Vec3& v1, const Vec3& v2);