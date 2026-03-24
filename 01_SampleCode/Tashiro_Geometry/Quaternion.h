#pragma once
#include "Vec3.h"

/// <summary>
/// クオータニオン
/// </summary>
struct Quaternion
{
public:
	float x;
	float y;
	float z;
	float w;

	Quaternion();
	Quaternion(float x, float y, float z, float w);

	/// <summary>
	/// 共役クオータニオン
	/// </summary>
	/// <returns>共役クオータニオン</returns>
	Quaternion Conjugated() const;

	/* 演算子オーバーロード */
	Quaternion operator* (const Quaternion& q) const;
	Vec3 operator* (const Vec3& vec) const;

	/// <summary>
	/// クオータニオンをX,Y,Zにどれくらい回転しているかにして返す
	/// </summary>
	/// <returns>弧度法での回転具合</returns>
	Vec3 GetRadian() const;
};

/// <summary>
/// 軸を基準に回転させるクオータニオンを作成
/// </summary>
/// <param name="angle">回転度合い(度数法)</param>
/// <param name="axis">軸</param>
/// <returns>クオータニオン</returns>
Quaternion AngleAxis(float angle, const Vec3& axis);
/// <summary>
/// クオータニオン同士の内積
/// </summary>
/// <param name="q1">右辺</param>
/// <param name="q2">左辺</param>
/// <returns>内積</returns>
float Dot(const Quaternion& q1, const Quaternion& q2);

