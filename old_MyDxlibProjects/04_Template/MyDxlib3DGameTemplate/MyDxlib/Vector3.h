#pragma once
#include <cmath>
#include <cassert>
#include <string>
#include <numbers> // π用
#include <algorithm> // clamp用
#include "DxLib.h"

struct Vector3
{
	/* メンバ変数 */
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;

	/* staticメンバ変数宣言 */
	static const Vector3 Zero;
	static const Vector3 Left;
	static const Vector3 Right;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Forward;
	static const Vector3 Back;

	/* メンバ関数 */
	// ベクトルの長さの2乗
	float LengthSq() const {
		return X * X + Y * Y + Z * Z;
	}
	// ベクトルの長さ
	float Length() const {
		return std::sqrtf(LengthSq());
	}
	// 正規化
	Vector3 Normalize() const {
		// 長さの2乗を出す 
		float lenSq = LengthSq();
		// 0以下ならば、0ベクトルを返す(リリース時クラッシュ回避)
		if (lenSq <= 0.0f) {
			assert(lenSq != 0 && "Vector3::Normalize : 0 division");// Debugビルドで0除算を検出
			return Zero;
		}
		// 長さを出す
		float len = std::sqrtf(lenSq);
		// 長さを成分で割って、正規化した値を返す
		return { X / len, Y / len, Z / len };
	}
	// 絶対値
	Vector3 Abs() const {
		return { std::abs(X), std::abs(Y), std::abs(Z) };
	}
	// 文字列変換
	std::string ToString() const {
		return std::to_string(X) + ":" + std::to_string(Y) + ":" + std::to_string(Z);
	}

	/* staticメンバ関数 */
	// 内積
	static float Dot(const Vector3& l, const Vector3& r) {
		return l.X * r.X + l.Y * r.Y + l.Z * r.Z;
	}
	// 外積
	static Vector3 Cross(const Vector3& l, const Vector3& r) {
		return {
			l.Y * r.Z - l.Z * r.Y,
			l.Z * r.X - l.X * r.Z,
			l.X * r.Y - l.Y * r.X
		};
	}
	// 距離
	static float Distance(const Vector3& from, const Vector3& to) {
		return (to - from).Length();
	}
	//反射ベクトル(入射ベクトルv、法線ベクトルnormal)
	static Vector3 Reflect(const Vector3& v, const Vector3& normal) {
		return v - 2.0f * Dot(v, normal) * normal;
	}
	//平面投射(ベクトルvを法線ベクトルnormalの平面に投射)(壁ずり用)
	static Vector3 ProjectOnPlane(const Vector3& v, const Vector3& normal) {
		return v - normal * Dot(v, normal);
	}
	//なす角(度数法)
	static float Angle(const Vector3& from, const Vector3& to) {
		// ベクトルの長さの積を出す
		float len = std::sqrtf(from.LengthSq() * to.LengthSq());
		// 0以下ならば、0度を返す(リリース時クラッシュ回避)
		if (len <= 0.0f) return 0.0f;
		// 内積を長さの積で割って、なす角の余弦(cos)を出す(-1~1に制限)
		float dot = std::clamp(Dot(from, to) / len, -1.0f, 1.0f);
		// 余弦からなす角を出す
		return std::acos(dot) * 180.0f / std::numbers::pi_v<float>;
	}
	//線形補間
	static Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
		return start + (end - start) * t;
	}

	/* 演算子オーバーロード */
	//単項演算子(const付けるとメンバ変数の変更を行えなくなる)
	//-A
	Vector3 operator-() const { return { -X, -Y, -Z }; }
	//A+B
	Vector3 operator+(const Vector3& r) const { return { X + r.X, Y + r.Y, Z + r.Z }; }
	//A-B
	Vector3 operator-(const Vector3& r) const { return { X - r.X, Y - r.Y, Z - r.Z }; }
	//A*k
	Vector3 operator*(float k)          const { return { X * k, Y * k, Z * k }; }
	//A/k
	Vector3 operator/(float k)          const {
		assert(k != 0 && "Vector3::operator/ : 0 division");
		return { X / k, Y / k, Z / k };
	}
	//複合代入演算子
	// A+=B
	Vector3& operator+=(const Vector3& r) { X += r.X; Y += r.Y; Z += r.Z; return *this; }
	// A-=B
	Vector3& operator-=(const Vector3& r) { X -= r.X; Y -= r.Y; Z -= r.Z; return *this; }
	// A*=k
	Vector3& operator*=(float k) { X *= k; Y *= k; Z *= k; return *this; }
	//friend関数(非メンバ関数)として定義することで、左辺が定数の演算も可能になる
	// k*A
	friend Vector3 operator*(float k, const Vector3& r) { return r * k; }
	//DXlibのVECTOR型に自動変換
	operator VECTOR() const
	{
		return VGet(X, Y, Z);
	}
	//DXlibのVECTOR型を代入できるようにする
	Vector3& operator=(const VECTOR& v)
	{
		X = v.x;
		Y = v.y;
		Z = v.z;
		return *this;
	}
};

/* staticメンバ変数の実装 */
inline const Vector3 Vector3::Zero = { 0.0f,  0.0f,  0.0f };
inline const Vector3 Vector3::Left = { -1.0f,  0.0f,  0.0f };
inline const Vector3 Vector3::Right = { 1.0f,  0.0f,  0.0f };
inline const Vector3 Vector3::Up = { 0.0f,  1.0f,  0.0f };//3D空間では、上方向はY軸が正の方向
inline const Vector3 Vector3::Down = { 0.0f, -1.0f,  0.0f };//3D空間では、下方向はY軸が負の方向
inline const Vector3 Vector3::Forward = { 0.0f,  0.0f,  -1.0f };//DxLibの3D空間では、前方向はZ軸が負の方向
inline const Vector3 Vector3::Back = { 0.0f,  0.0f, 1.0f };//DxLibの3D空間では、後方向はZ軸が正の方向