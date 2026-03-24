#pragma once
#include"Vec3.h"
#include"Quaternion.h"

Vec3 GetNearestPtOnBox(const Vec3& pos, const Vec3& boxPos, const Vec3& boxScale, const Quaternion& rot)
{
	// ボックスの中心から円の中心までのベクトルを作成
	auto boxToPt = pos - boxPos;

	// それぞれの方向(正面・右・上)へのベクトルを作成する
	auto right = rot * Vec3::Right();
	auto up = rot * Vec3::Up();
	auto front = rot * Vec3::Front();
	auto size = boxScale;
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

