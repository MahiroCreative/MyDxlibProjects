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
	/// 逆クォータニオンを返す。ただし、三次元なので共役を出してるだけ。
	/// (虚部だけマイナスにする)
	/// </summary>
	Quaternion Inverse() const { return Quaternion{ -this->X,-this->Y,-this->Z,this->W }; }
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
		//return
		return ans;
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
		//度数からラジアンに変換
		angle.X = angle.X * std::numbers::pi_v<float> / 180.0f;
		angle.Y = angle.Y * std::numbers::pi_v<float> / 180.0f;
		angle.Z = angle.Z * std::numbers::pi_v<float> / 180.0f;

		//要素計算
		float cosX = std::cos(angle.X / 2);
		float sinX = std::sin(angle.X / 2);
		float cosY = std::cos(angle.Y / 2);
		float sinY = std::sin(angle.Y / 2);
		float cosZ = std::cos(angle.Z / 2);
		float sinZ = std::sin(angle.Z / 2);

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
		//度数からラジアンに変換
		float Rad = angle * std::numbers::pi_v<float> / 180.0f;

		//回転軸の正規化
		Vector3 nAxis = axis.Normalize();
		//存在しうる軸かを確認.
		assert(nAxis.LengthSq() > 0.0f && "軸がありません");

		//回転クォータニオンの作成
		//(w:実部 , ijk:虚部 とする)
		//n: ijk軸上の単位ベクトル(軸となるベクトル)
		//q = w cos(θ/2) + n sin(θ/2)
		Quaternion ans;
		ans.W = std::cos(Rad * 0.5f);
		ans.X = nAxis.X * std::sin(Rad * 0.5f);
		ans.Y = nAxis.Y * std::sin(Rad * 0.5f);
		ans.Z = nAxis.Z * std::sin(Rad * 0.5f);

		//return
		return ans;
	}

	/*演算子オーバーロード*/
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
		Quaternion q{ right.X,right.Y,right.Z,0.0f };

		//回転クォータニオンのインバースの作成(逆クォータニオンを出すのは大変なので)
		Quaternion qInv = Inverse();

		//回転後のクォータニオンの作成
		Quaternion qRes = (*this) * q * qInv;

		//3次元座標に変換
		Vector3 ans{ qRes.X, qRes.Y, qRes.Z };

		//return.
		return ans;
	}

};

/*staticメンバ変数の実装*/
inline const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };