#include "MathHelp.h"
#include <cmath>
#include <algorithm>
#include "Vec2.h"
#include "Common/Game.h"

bool IsParallelOnLine(const Vec3& startA, const Vec3& endA, const Vec3& startB, const Vec3& endB)
{
	auto vA = endA - startA;
	auto vB = endB - startB;

	auto cross = Cross(vA, vB);

	return cross.SqLength() < 0.001f;
}

bool IsCrossSurfaceOnLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm)
{
	auto centerToStart = start - center;
	auto centerToEnd = end - center;

	// 両方とも鋭角ならば面をまたいでいない
	return (Dot(norm, centerToStart) * Dot(norm, centerToEnd) < 0);
}

Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end)
{
	float t = 0.0f;
	return GetNearestPtOnLine(point, start, end, t);
}

Vec3 GetNearestPtOnLine(const Vec3& point, const Vec3& start, const Vec3& end, float& rate)
{
	Vec3 startToEnd = end - start;
	Vec3 startToPoint = point - start;

	// 線上のどの辺か
	rate = Dot(startToEnd, startToPoint) / startToEnd.SqLength();
	// 排他処理
	rate = std::fmax(std::fmin(rate, 1.0f), 0.0f);

	return start + startToEnd * rate;
}

void GetNearestPtOnLine(const Vec3& centerA, const Vec3& dirA, const Vec3& centerB, const Vec3& dirB, Vec3& outPtA, Vec3& outPtB)
{
	auto aS = centerA + dirA;
	auto aE = centerA - dirA;
	auto bS = centerB + dirB;
	auto bE = centerB - dirB;

	outPtA = GetNearestPtOnLine(centerB, aS, aE);
	outPtB = GetNearestPtOnLine(outPtA, bS, bE);
}

Vec3 GetNearestPtOnBox(const Vec3& pos, const Vec3& boxPos, const Vec3& boxScale, const Quaternion& rot)
{
	// ボックスの中心から円の中心までのベクトルを作成
	auto boxToPt = pos - boxPos;

	// それぞれの方向(正面・右・上)へのベクトルを作成する
	auto right = rot * Vec3::Right();
	auto up = rot * Vec3::Up();
	auto front = rot * Vec3::Front();
	auto size = boxScale * 0.5f;
	auto prRight = Projection(boxToPt, right);
	auto prUp = Projection(boxToPt, up);
	auto prFront = Projection(boxToPt, front);
	// 向きをサイズ以内にする
	if (prRight.SqLength() > size.x * size.x) prRight = prRight.GetNormalized() * size.x;
	if (prUp.SqLength() > size.y * size.y)    prUp = prUp.GetNormalized() * size.y;
	if (prFront.SqLength() > size.z * size.z) prFront = prFront.GetNormalized() * size.z;
	// 球に近い場所を求める
	return boxPos + prRight + prUp + prFront;
}

bool IsNearestPointOnLine(const Vec3& point, const Vec3& start, const Vec3& end)
{
	Vec3 startToEnd = end - start;
	Vec3 startToPoint = point - start;

	// 線上のどの辺か
	float t = Dot(startToEnd, startToPoint) / startToEnd.SqLength();

	// tが線分上ならtrue
	return 0.0f <= t && t <= 1.0f;
}



Vec3 GetCrossPtSurfaceAndLine(const Vec3& start, const Vec3& end, const Vec3& center, const Vec3& norm)
{
	auto centerToStart = start - center;
	auto centerToEnd = end - center;

	// 線のスタート・ゴールと面までの長さを取得
	auto startSurfaceLen = Projection(centerToStart, norm).Length();
	auto endSurfaceLen = Projection(centerToEnd, norm).Length();
	// 交点が線分上の何処あたりかを計算
	auto rate = startSurfaceLen / (startSurfaceLen + endSurfaceLen);
	// 交点を取得
	return centerToStart * (1 - rate) + centerToEnd * rate + center;
}

Vec3 GetCrossPtSurfaceAndPt(const Vec3& pt, const Vec3& center, const Vec3& norm)
{
	auto ptToCenter = center - pt;
	auto proj = Projection(ptToCenter, norm);
	return pt + proj;
}


Quaternion GetQuaternion(const Vec3& v1, const Vec3& v2)
{
	auto norm1 = v1.GetNormalized();
	auto norm2 = v2.GetNormalized();
	float dot = Dot(norm1, norm2);
	float angle = std::acosf(dot) * Game::RAD_2_DEG;
	auto axis = Cross(norm1, norm2);
	if (axis.SqLength() == 0.0f)
	{
		axis = norm1;
	}

	return AngleAxis(angle, axis);
}