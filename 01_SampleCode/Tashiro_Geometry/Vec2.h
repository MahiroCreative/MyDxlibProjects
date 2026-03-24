#pragma once
#include <DxLib.h>

/// <summary>
/// ベクター2
/// </summary>
struct Vec2
{
public:
	float x;
	float y;
	
	Vec2();
	Vec2(float inX, float inY);

	/* 演算 */
	Vec2 operator-() const;
	Vec2 operator+(const Vec2& val) const;
	void operator+=(const Vec2& val);
	Vec2 operator-(const Vec2& val) const;
	void operator-=(const Vec2& val);
	Vec2 operator*(float scale) const;
	void operator*=(float scale);
	Vec2 operator/(float scale) const;
	void operator/=(float scale);
	bool operator==(const Vec2& val) const;
	bool operator!=(const Vec2& val) const;

	/// <summary>
	/// 長さ取得
	/// </summary>
	/// <returns></returns>
	float Length() const;
	/// <summary>
	/// 長さの2乗を取得
	/// </summary>
	/// <returns></returns>
	float SqLength() const;
	/// <summary>
	/// 正規化したベクトルを取得
	/// </summary>
	/// <returns></returns>
	Vec2 GetNormalized() const;
	/// <summary>
	/// 自身の正規化
	/// </summary>
	void Normalize();

	/// <summary>
	/// DxLib用のVECTORを取得
	/// </summary>
	/// <returns>DxLib::VECTOR</returns>
	DxLib::VECTOR VGet() const;
};

/// <summary>
/// 内積
/// </summary>
/// <param name="item1">ベクトル１</param>
/// <param name="item2">ベクトル２</param>
/// <returns>内積結果</returns>
float Dot(const Vec2& item1, const Vec2& item2);
/// <summary>
/// 外積
/// </summary>
/// <param name="item1">ベクトル１</param>
/// <param name="item2">ベクトル２</param>
/// <returns>外積結果</returns>
float Cross(const Vec2& item1, const Vec2& item2);
/// <summary>
/// 射影
/// </summary>
/// <param name="projection">射影するベクトル</param>
/// <param name="base">射影後の方向となるベクトル</param>
/// <returns>射影ベクトル</returns>
Vec2 Projection(const Vec2& projection, const Vec2& base);
/// <summary>
/// 同じ方向を向いているか(誤差許容)
/// </summary>
/// <param name="dir1">方向ベクトル１</param>
/// <param name="dir2">方向ベクトル２</param>
/// <returns>true:ほぼ同じ方向を向いている/ false:同じ方向を向いていない</returns>
bool IsSameDirection(const Vec2& dir1, const Vec2& dir2);
