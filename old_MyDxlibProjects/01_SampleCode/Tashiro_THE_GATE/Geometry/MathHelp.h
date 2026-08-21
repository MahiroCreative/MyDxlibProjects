#pragma once
#include "Vec3.h"
#include "Quaternion.h"

/// <summary>
/// 線分同士が平行か判定
/// </summary>
/// <param name="startA">線分Aの開始地点</param>
/// <param name="endA">線分Aの終了地点</param>
/// <param name="startB">線分Bの開始地点</param>
/// <param name="endB">線分Bの終了地点</param>
/// <returns>true: 平行/ false: 平行じゃない</returns>
bool IsParallelOnLine(const Vec3& startA, const Vec3& endA, const Vec3& startB, const Vec3& endB);
/// <summary>
/// 線分が面をまたいでいるか判定
/// </summary>
/// <param name="start">線分の開始地点</param>
/// <param name="end">線分の終了地点</param>
/// <param name="center">面の中心点</param>
/// <param name="norm">法線</param>
/// <returns>true:またいでいる/ false:またいでいない</returns>
bool IsCrossSurfaceOnLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm);

/// <summary>
/// 最近接点を取得
/// </summary>
/// <param name="point">点</param>
/// <param name="start">線分の開始地点</param>
/// <param name="end">線分の終了地点</param>
/// <returns>最近接点</returns>
Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end);
/// <summary>
/// 最近接点を取得
/// </summary>
/// <param name="point">点</param>
/// <param name="start">線分の開始地点</param>
/// <param name="end">線分の終了地点</param>
/// <param name="rate">線分上での割合</param>
/// <returns>最近接点</returns>
Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end, float& rate);
/// <summary>
/// 線分同士の最近接点を取得
/// </summary>
/// <param name="centerA">線分Aの中心点</param>
/// <param name="dirA">線分Aの方向</param>
/// <param name="centerB">線分Bの中心点</param>
/// <param name="dirB">線分Bの方向</param>
/// <param name="outPtA">線分A上での最近接点</param>
/// <param name="outPtB">線分B上での最近接点</param>
void GetNearestPtOnLine(const Vec3& centerA, const Vec3& dirA, const Vec3& centerB, const Vec3& dirB, Vec3& outPtA, Vec3& outPtB);
/// <summary>
/// ボックス上での最近接点を取得
/// </summary>
/// <param name="pos">点</param>
/// <param name="boxPos">ボックスの中心点</param>
/// <param name="boxScale">ボックスのサイズ</param>
/// <param name="rot">ボックスの回転</param>
/// <returns>最近接点</returns>
Vec3 GetNearestPtOnBox(const Vec3& pos, const Vec3& boxPos, const Vec3& boxScale, const Quaternion& rot);
/// <summary>
/// 線分上に最近接点があるかの判定
/// </summary>
/// <param name="point">点</param>
/// <param name="start">線分の開始地点</param>
/// <param name="end">線分の終了地点</param>
/// <returns>最近接点</returns>
bool IsNearestPointOnLine(const Vec3& point, const Vec3& start, const Vec3& end);


/// <summary>
/// 線と面の交点を取得
/// </summary>
/// <param name="start">線分の開始位置</param>
/// <param name="end">線分の終了位置</param>
/// <param name="center">面の中心</param>
/// <param name="norm">法線</param>
/// <returns>最近接点</returns>
Vec3 GetCrossPtSurfaceAndLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm);
/// <summary>
/// 点と面の交点を取得
/// </summary>
/// <param name="pt">点</param>
/// <param name="center">面の中心</param>
/// <param name="norm">法線</param>
/// <returns>最近接点</returns>
Vec3 GetCrossPtSurfaceAndPt(const Vec3& pt, const Vec3& center, const Vec3& norm);
