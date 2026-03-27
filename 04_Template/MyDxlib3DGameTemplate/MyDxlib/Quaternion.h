#pragma once
//数学系
//C++標準ライブラリ
#include <cassert>
#include <cmath>
#include <numbers>
#include <algorithm>
//original
#include "Vector3.h"

/// <summary>
/// Quaternion型の構造体(float)
/// </summary>
struct Quaternion
{
	/*メンバ変数*/
	float X = 0, Y = 0, Z = 0, W = 1.0f;//虚部X,虚部Y,虚部Z,実部W

	/*staticメンバ変数宣言(実装は外で行う)*/
	static const Quaternion Identity;//単位クォータニオン(回転なし)

	/*メンバ関数*/
	/// <summary>
	/// クォータニオンの成分をセット。
	/// </summary>
	void Set(float x, float y, float z, float w) { this->X = x; this->Y = y; this->Z = z; this->W = w; }
	/// <summary>
	/// クォータニオンの長さの2乗を返す。(normalize用)
	/// </summary>
	/// <returns></returns>
	float LengthSq() const { return X * X + Y * Y + Z * Z + W * W; }
	/// <summary>
	/// クォータニオンの長さを返す。(normalize用)
	/// </summary>
	/// <returns></returns>
	float Length()   const { return std::sqrtf(LengthSq()); }
	/// <summary>
	/// クォータニオンの長さを1にする.(1でないと正常に回転しない)
	/// </summary>
	/// <returns></returns>
	Quaternion Normalize() const {
		float lenSq = LengthSq();
		if (lenSq <= 0.0f) return Identity;//リリース時クラッシュ回避
		float len = std::sqrtf(lenSq);
		Quaternion res;
		res.X = X / len; res.Y = Y / len; res.Z = Z / len; res.W = W / len;
		return res;
	}
	/// <summary>
	/// 共役クォータニオンを返す(虚数成分のみ反転)。
	/// Normalizeしておけば、逆クォータニオンとして使える(こちらの方が圧倒的に早い)。
	/// </summary>
	/// <returns></returns>
	Quaternion Conjugate() const {
		Quaternion res;
		res.X = -X; res.Y = -Y; res.Z = -Z; res.W = W;
		return res;
	}
	/// <summary>
	/// 数学的に厳密な逆クォータニオン（基本は使わず、共役で済ます）
	/// </summary>
	Quaternion Inverse() const {
		float lenSq = LengthSq();
		if (lenSq <= 0.0f) return Identity;
		return Quaternion{ -X / lenSq, -Y / lenSq, -Z / lenSq, W / lenSq };
	}
	/// <summary>
	/// オイラー角に変換。
	/// </summary>
	/// <returns>オイラー角(Vector3)</returns>
	Vector3 GetEulerAngle() const
	{
		//変数
		Vector3 ans;
		//計算
		ans.X = std::atan2(2 * (this->W * this->X + this->Y * this->Z), 1 - 2 * (this->X * this->X + this->Y * this->Y));
		ans.Y = std::asin(std::clamp(2 * (this->W * this->Y - this->Z * this->X), -1.0f, 1.0f));//誤差対策
		ans.Z = std::atan2(2 * (this->W * this->Z + this->X * this->Y), 1 - 2 * (this->Y * this->Y + this->Z * this->Z));
		//ラジアン→度数
		float toDeg = 180.0f / std::numbers::pi_v<float>;
		//return
		return { ans.X * toDeg, ans.Y * toDeg, ans.Z * toDeg };
	}
	/// <summary>
	/// 成分を文字列にして返す.
	/// </summary>
	std::string ToString() const
	{
		return std::to_string(X) + ":" + std::to_string(Y) + ":" + std::to_string(Z) + ":" + std::to_string(W);
	}

	/*staticメンバ関数*/
	/// <summary>
	/// オイラー角からクォータニオンを作成
	/// (XYZ系)
	/// </summary>
	/// <param name="angle">オイラー角(Vector3)</param>
	static Quaternion Euler(Vector3 angle)
	{
		// 度数→ラジアン かつ /2 を一括計算
		float toRad = std::numbers::pi_v<float> / 360.0f;

		//度数からラジアンに変換
		float radX = angle.X * toRad;
		float radY = angle.Y * toRad;
		float radZ = angle.Z * toRad;

		//要素計算
		float cosX = std::cos(radX);
		float sinX = std::sin(radX);
		float cosY = std::cos(radY);
		float sinY = std::sin(radY);
		float cosZ = std::cos(radZ);
		float sinZ = std::sin(radZ);

		//クォータニオンの作成
		Quaternion ans;
		ans.W = cosX * cosY * cosZ + sinX * sinY * sinZ;
		ans.X = sinX * cosY * cosZ - cosX * sinY * sinZ;
		ans.Y = cosX * sinY * cosZ + sinX * cosY * sinZ;
		ans.Z = cosX * cosY * sinZ - sinX * sinY * cosZ;

		//return
		return ans;
	}
	/// <summary>
	/// クオータニオンの内積
	/// </summary>
	static float Dot(const Quaternion& left, const Quaternion& right)
	{
		return left.X * right.X + left.Y * right.Y + left.Z * right.Z + left.W * right.W;
	}
	/// <summary>
	/// 軸を基準に回転させるクオータニオンを作成.
	/// </summary>
	/// <param name="angle">回転度合い(度数法)</param>
	/// <param name="axis">軸(Vector3)</param>
	/// <returns>クオータニオン</returns>
	static Quaternion AngleAxis(float angle, const Vector3& axis)
	{
		// 修正箇所：引数の angle をラジアンの半分に変換する
		float halfRad = (angle * std::numbers::pi_v<float>) / 360.0f;

		// 回転軸の正規化
		Vector3 nAxis = axis.Normalize();
		assert(nAxis.LengthSq() > 0.0f && "軸がありません");

		// 回転クォータニオンの作成
		// q = ( n * sin(θ/2), cos(θ/2) )
		float s = std::sin(halfRad);
		Quaternion ans;
		ans.X = nAxis.X * s;
		ans.Y = nAxis.Y * s;
		ans.Z = nAxis.Z * s;
		ans.W = std::cos(halfRad);

		return ans;
	}
	/// <summary>
	/// Slerp(球面線形補間) : クォータニオンならではのLerp。回転を滑らかに繋ぐ。
	/// </summary>
	/// <param name="q1"></param>
	/// <param name="q2"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) 
	{
		// 内積を出す
		float dot = Dot(q1, q2);
		Quaternion target = q2;

		// 最短経路（鋭角側）を通るための反転
		if (dot < 0.0f) {
			dot = -dot;
			target.X = -q2.X; target.Y = -q2.Y; target.Z = -q2.Z; target.W = -q2.W;
		}

		// 角度が極めて小さい場合は線形補間で代用（0除算回避）
		if (dot >= 1.0f - 0.0005f) {
			Quaternion res;
			res.X = q1.X + (target.X - q1.X) * t;
			res.Y = q1.Y + (target.Y - q1.Y) * t;
			res.Z = q1.Z + (target.Z - q1.Z) * t;
			res.W = q1.W + (target.W - q1.W) * t;
			return res.Normalize();
		}

		// 内積から角度を出す
		float theta = std::acos(dot);
		float sinTheta = std::sin(theta);
		float scale1 = std::sin(theta * (1.0f - t)) / sinTheta;
		float scale2 = std::sin(theta * t) / sinTheta;

		// 補間してクォータニオンを出す
		Quaternion res;
		res.X = scale1 * q1.X + scale2 * target.X;
		res.Y = scale1 * q1.Y + scale2 * target.Y;
		res.Z = scale1 * q1.Z + scale2 * target.Z;
		res.W = scale1 * q1.W + scale2 * target.W;
		return res;
	}

	/*演算子オーバーロード*/
	// 単項演算子（符号反転）
	Quaternion operator-() const {
		return { -X, -Y, -Z, -W };
	}
	/// <summary>
	/// クォータニオン同士の掛け算(回転の合成)
	/// </summary>
	Quaternion operator*(const Quaternion& right) const
	{
		return{
			this->W * right.X + this->X * right.W + this->Y * right.Z - this->Z * right.Y,
			this->W * right.Y - this->X * right.Z + this->Y * right.W + this->Z * right.X,
			this->W * right.Z + this->X * right.Y - this->Y * right.X + this->Z * right.W,
			this->W * right.W - this->X * right.X - this->Y * right.Y - this->Z * right.Z
		};
	}
	/// <summary>
	/// クォータニオンとベクトルの掛け算。回転後のベクトルを返す。
	/// (計算としてはqpq*であり、クォータニオンと共役クォータニオンでベクトルを挟んでる)
	/// </summary>
	Vector3 operator*(const Vector3& right) const
	{
		//３次元ベクトルを純粋クォータニオンに変換
		Quaternion q{ right.X,right.Y,right.Z,0.0f};

		//自分自身を正規化しておく(回転クォータニオンは長さ1でないと正常に回転しない)
		Quaternion qNorm = Normalize();

		//回転クォータニオンのインバースの作成(逆クォータニオンを出すのは大変なので)
		Quaternion qInv = qNorm.Conjugate();

		//回転後のクォータニオンの作成
		Quaternion qRes = qNorm * q * qInv;

		//3次元座標に変換
		Vector3 ans{ qRes.X, qRes.Y, qRes.Z };

		//return.
		return ans;
	}

};

/*staticメンバ変数の実装*/
inline const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };