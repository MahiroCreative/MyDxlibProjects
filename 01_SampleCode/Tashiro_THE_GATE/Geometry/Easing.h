#pragma once
#include <cmath>
#include <algorithm>
#include "Common/Game.h"
#include "Vec3.h"
#include "Quaternion.h"

enum class EasingType
{
	LERP,
	EASE_IN,
	EASE_OUT,
	EASE_IN_OUT,
	SLERP,
	MAX
};

class Easing
{
public:
	/// <summary>
	/// 線形補間
	/// </summary>
	/// <typeparam name="T">型</typeparam>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <returns>開始値＋計算結果値</returns>
	template <typename T>
	static T Lerp(T start, T end, float t)
	{
		return static_cast<T>(start + (end - start) * t);
	}
	/// <summary>
	/// 線形補間(ベクトルver.)
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <returns>開始値＋計算結果値</returns>
	static Vec3 Lerp(const Vec3& start, const Vec3& end, float t)
	{
		return start + (end - start) * t;
	}
	/// <summary>
	/// 線形補間 0に近いほど動きが小さく1に近いほど動きが大きい
	/// </summary>
	/// <typeparam name="T">型</typeparam>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	template <typename T>
	static T EaseIn(T start, T end, float t, float effect = 2.0f)
	{
		float rate = std::powf(t, effect);
		return Lerp(start, end, rate);
	}
	/// <summary>
	/// 線形補間 0に近いほど動きが小さく1に近いほど動きが大きい(ベクトルver.)
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	static Vec3 EaseIn(const Vec3& start, const Vec3& end, float t, float effect = 2.0f)
	{
		float rate = std::powf(t, effect);
		return Lerp(start, end, rate);
	}
	/// <summary>
	/// 線形補間 0に近いほど動きが大きく1に近いほど動きが小さい
	/// </summary>
	/// <typeparam name="T">型</typeparam>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	template <typename T>
	static T EaseOut(T start, T end, float t, float effect = 2.0f)
	{
		float rate = 1.0f - std::powf(1.0f - t, effect);
		return Lerp(start, end, rate);
	}
	/// <summary>
	/// 線形補間 0に近いほど動きが大きく1に近いほど動きが小さい(ベクトルver.)
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	static Vec3 EaseOut(const Vec3& start, const Vec3& end, float t, float effect = 2.0f)
	{
		float rate = 1.0f - std::powf(1.0f - t, effect);
		return Lerp(start, end, rate);
	}
	/// <summary>
	/// 線形補間 真ん中が動きが小さく両端が動きが大きい
	/// </summary>
	/// <typeparam name="T">型</typeparam>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	template <typename T>
	static T EaseInOut(T start, T end, float t, float effect = 2.0f)
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
	/// <summary>
	/// 線形補間 真ん中が動きが小さく両端が動きが大きい(ベクトルver.)
	/// </summary>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="t">割合</param>
	/// <param name="effect">影響度合い</param>
	/// <returns>開始値＋計算結果値</returns>
	static Vec3 EaseInOut(const Vec3& start, const Vec3& end, float t, float effect = 2.0f)
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

	/// <summary>
	/// 球面線形補間(ベクトルver.)
	/// </summary>
	/// <param name="v1">開始値</param>
	/// <param name="v2">終了値</param>
	/// <param name="t">割合</param>
	/// <returns>開始値から割合分回転したベクトル</returns>
	static Vec3 Slerp(const Vec3& v1, const Vec3& v2, float t)
	{
		float dot = Vec3::Dot(v1.GetNormalized(), v2.GetNormalized());
		if (dot > 0.9999f) return v1;
		auto q = Quaternion::GetQuaternion(v1.GetNormalized(), v2.GetNormalized());

		return q * v1;
	}
	/// <summary>
	/// 球面線形補間(クオータニオンver.)
	/// </summary>
	/// <param name="q1">開始値</param>
	/// <param name="q2">終了値</param>
	/// <param name="t">割合</param>
	/// <returns>開始値から割合分回転したクオータニオン</returns>
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t)
	{
		float dot = Quaternion::Dot(q1, q2);

		if (dot >= 1.0f)
		{
			return q1;
		}

		float angle = std::acosf(dot);

		angle = std::abs(angle);

		float st = std::sinf(angle);

		float s1 = std::sinf(angle * t) / st;
		float s2 = std::sinf(angle * (1.0f - t)) / st;

		Quaternion res;
		res.x = s1 * q1.x + s2 * q2.x;
		res.y = s1 * q1.y + s2 * q2.y;
		res.z = s1 * q1.z + s2 * q2.z;
		res.w = s1 * q1.w + s2 * q2.w;

		assert(!isnan(res.x));
		assert(!isnan(res.y));
		assert(!isnan(res.z));

		return res;
	}
};

