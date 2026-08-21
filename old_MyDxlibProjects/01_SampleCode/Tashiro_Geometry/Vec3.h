#pragma once
#include <DxLib.h>
#include "Vec2.h"

/// <summary>
/// ベクター3
/// </summary>
struct Vec3
{
public:
	float x;
	float y;
	float z;

	Vec3();
	Vec3(float inX, float inY, float inZ);
	Vec3(const Vec2& vec2);

	/// <summary>
	/// 右ベクトル
	/// </summary>
	/// <returns>(1, 0, 0)</returns>
	static Vec3 Right();
	/// <summary>
	/// 左ベクトル
	/// </summary>
	/// <returns>(-1, 0, 0)</returns>
	static Vec3 Left();
	/// <summary>
	/// 上ベクトル
	/// </summary>
	/// <returns>(0, 1, 0)</returns>
	static Vec3 Up();
	/// <summary>
	/// 下ベクトル
	/// </summary>
	/// <returns>(0, -1, 0)</returns>
	static Vec3 Down();
	/// <summary>
	/// 正面ベクトル
	/// </summary>
	/// <returns>(0, 0, 1)</returns>
	static Vec3 Front();
	/// <summary>
	/// 背面ベクトル
	/// </summary>
	/// <returns>(0, 0, -1)</returns>
	static Vec3 Back();

	/* 演算 */
	Vec3 operator-() const;
	Vec3 operator+(const Vec3& val) const;
	void operator+=(const Vec3& val);
	Vec3 operator-(const Vec3& val) const;
	void operator-=(const Vec3& val);
	Vec3 operator*(float scale) const;
	void operator*=(float scale);
	Vec3 operator/(float scale) const;
	void operator/=(float scale);
	bool operator==(const Vec3& val) const;
	bool operator!=(const Vec3& val) const;

	/// <summary>
	/// XZ方向のみを取得
	/// </summary>
	/// <returns>(x, z)</returns>
	Vec2 GetXZDir() const;

	/// <summary>
	/// 長さ取得
	/// </summary>
	/// <returns>長さ</returns>
	float Length() const;
	/// <summary>
	/// 長さの2乗を取得
	/// </summary>
	/// <returns>長さの2乗</returns>
	float SqLength() const;
	/// <summary>
	/// 正規化したベクトルを取得
	/// </summary>
	/// <returns>正規化ベクトル</returns>
	Vec3 GetNormalized() const;
	/// <summary>
	/// 自身の正規化
	/// </summary>
	void Normalize();
	/// <summary>
	/// 自身の絶対値化
	/// </summary>
	void Abs();

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
float Dot(const Vec3& item1, const Vec3& item2);
/// <summary>
/// 外積
/// </summary>
/// <param name="item1">ベクトル１</param>
/// <param name="item2">ベクトル２</param>
/// <returns>外積結果</returns>
Vec3 Cross(const Vec3& item1, const Vec3& item2);
/// <summary>
/// 射影
/// </summary>
/// <param name="projection">射影するベクトル</param>
/// <param name="base">射影後の方向となるベクトル</param>
/// <returns>射影ベクトル</returns>
Vec3 Projection(const Vec3& projection, const Vec3& base);
/// <summary>
/// 反射
/// </summary>
/// <param name="vec">反射させるベクトル</param>
/// <param name="norm">法線ベクトル</param>
/// <returns>反射後のベクトル</returns>
Vec3 Reflection(const Vec3& vec, const Vec3& norm);
/// <summary>
/// 同じ方向を向いているか(誤差許容)
/// </summary>
/// <param name="dir1">方向ベクトル１</param>
/// <param name="dir2">方向ベクトル２</param>
/// <returns>true:ほぼ同じ方向を向いている/ false:同じ方向を向いていない</returns>
bool IsSameDirection(const Vec3& dir1, const Vec3& dir2);
/// <summary>
/// 同じ方向を向いているか(180度未満)
/// </summary>
/// <param name="dir1">方向ベクトル１</param>
/// <param name="dir2">方向ベクトル２</param>
/// <returns>true:2つのベクトルの内積結果が180度未満/ false:内積結果が180度以上</returns>
bool IsUniformDirection(const Vec3& dir1, const Vec3& dir2);