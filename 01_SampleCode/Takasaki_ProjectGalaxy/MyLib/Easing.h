#pragma once
#include <cmath>
#include <algorithm>
#include "Vec3.h"
#include "Quaternion.h"

// 等速
template <typename T>
T Lerp(const T& start, const T& end, float t)
{
	return static_cast<T>(start + (end - start) * t);
}
Vec3 Lerp(const Vec3& start, const Vec3& end, float t);
// 加速
template <typename T>
T EaseIn(const T& start, const T& end, float t, float effect = 2.0f)
{
	float rate = std::powf(t, effect);
	return Lerp(start, end, rate);
}
Vec3 EaseIn(const Vec3& start, const Vec3& end, float t, float effect = 2.0f);
// 減速
template <typename T>
T EaseOut(const T& start, const T& end, float t, float effect = 2.0f)
{
	float rate = 1.0f - std::powf(1.0f - t, effect);
	return Lerp(start, end, rate);
}
Vec3 EaseOut(const Vec3& start, const Vec3& end, float t, float effect = 2.0f);
/// <summary>
/// 始めと終わりがゆるやか
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="start">始点</param>
/// <param name="end">終点</param>
/// <param name="t">進行度合</param>
/// <param name="effect">多いほど極端になる</param>
/// <returns></returns>
template <typename T>
T EaseInOut(const T& start, const T& end, float t, float effect = 2.0f)
{
	float rate;
	if (t < 0.5f)
	{
		rate = 2 * (effect - 1.0f) * std::powf(t, effect);
	}
	else
	{
		rate = 1.0f - std::powf(-2 * t + 2, effect) * 0.5f;
	}
	return Lerp(start, end, rate);
}

Vec3 EaseInOut(const Vec3& start, const Vec3& end, float t, float effect = 2.0f);

// 球面線形補間
Vec3 Slerp(const Vec3& v1, const Vec3& v2, float t);
Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);