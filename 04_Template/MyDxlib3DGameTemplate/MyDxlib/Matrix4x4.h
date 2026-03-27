#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include "Quaternion.h" // 追加
#include <string>
#include <cmath>
#include <numbers>// C++20の数学定数を使用
#include <algorithm>

/// <summary>
/// 数学準拠の4x4行列構造体
/// DxLibの行列への変換機能も持つ。
/// </summary>
struct Matrix4x4
{
	// [行][列] の順
	float m[4][4] = { 0 };

	/* staticメンバ変数宣言 */
	static const Matrix4x4 Identity; // 単位行列

	/* メンバ関数 */
	/// <summary>
	/// 転置行列を求める（行と列を入れ替える）
	/// </summary>
	Matrix4x4 Transpose() const
	{
		Matrix4x4 res;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				res.m[i][j] = m[j][i];
			}
		}
		return res;
	}
	/// <summary>
	/// 行列式 (Determinant) を計算する
	/// </summary>
	float Determinant() const {
		// 4x4行列の行列式（サラスの公式の4x4版のような展開）
		float s0 = m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
		float s1 = m[0][1] * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
		float s2 = m[0][2] * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
		float s3 = m[0][3] * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
		return s0 - s1 + s2 - s3;
	}
	/// <summary>
	/// 逆行列を「一から」計算する (余因子行列を使用)
	/// </summary>
	Matrix4x4 Inverse() const {
		float det = Determinant();
		if (std::abs(det) < 1e-6f) return Identity; // 逆行列が存在しない場合は単位行列を返す

		float invDet = 1.0f / det;
		Matrix4x4 res;

		// 各成分の余因子を計算
		// 0行目
		res.m[0][0] = (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])) * invDet;
		res.m[0][1] = -(m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])) * invDet;
		res.m[0][2] = (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) + m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1])) * invDet;
		res.m[0][3] = -(m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) + m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])) * invDet;

		// 1行目
		res.m[1][0] = -(m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])) * invDet;
		res.m[1][1] = (m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[0][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])) * invDet;
		res.m[1][2] = -(m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) + m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0])) * invDet;
		res.m[1][3] = (m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - m[0][2] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) + m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])) * invDet;

		// 2行目
		res.m[2][0] = (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) * invDet;
		res.m[2][1] = -(m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) * invDet;
		res.m[2][2] = (m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) - m[0][1] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) + m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])) * invDet;
		res.m[2][3] = -(m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) - m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) + m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])) * invDet;

		// 3行目
		res.m[3][0] = -(m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) * invDet;
		res.m[3][1] = (m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - m[0][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])) * invDet;
		res.m[3][2] = -(m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) - m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) + m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])) * invDet;
		res.m[3][3] = (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])) * invDet;

		return res;
	}
	// 文字列変換 
	std::string ToString() const
	{
		std::string str = "";
		for (int i = 0; i < 4; ++i) {
			str += std::to_string(m[i][0]) + ", " + std::to_string(m[i][1]) + ", "
				+ std::to_string(m[i][2]) + ", " + std::to_string(m[i][3]) + "\n";
		}
		return str;
	}


	/* staticメンバ関数 */
	/// <summary>
	/// 平行移動行列の作成
	/// </summary>
	/// <param name="pos">移動量(vector3)</param>
	/// <returns>平行移動行列</returns>
	static Matrix4x4 Translation(const Vector3& v)
	{
		Matrix4x4 res = Identity;
		res.m[0][3] = v.X; // 1行4列
		res.m[1][3] = v.Y; // 2行4列
		res.m[2][3] = v.Z; // 3行4列
		return res;
	}
	/// <summary>
	/// 拡大行列の作成
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static Matrix4x4 Scale(const Vector3& s)
	{
		Matrix4x4 res = Identity;
		res.m[0][0] = s.X; res.m[1][1] = s.Y; res.m[2][2] = s.Z;
		return res;
	}
	/// <summary>
	/// X軸周りの回転行列の作成
	/// </summary>
	/// <param name="degree">回転角度（度数法）。</param>
	/// <returns>X軸周りの回転を表す4x4行列。</returns>
	static Matrix4x4 RotationX(float degree)
	{
		float rad = degree * std::numbers::pi_v<float> / 180.0f;
		float c = std::cos(rad); float s = std::sin(rad);
		Matrix4x4 res = Identity;
		res.m[1][1] = c; res.m[1][2] = -s;
		res.m[2][1] = s; res.m[2][2] = c;
		return res;
	}
	/// <summary>
	/// Y軸周りの回転行列の作成。
	/// </summary>
	/// <param name="degree">回転角度（度数法）。</param>
	/// <returns>Y軸周りの回転を表す4x4行列。</returns>
	static Matrix4x4 RotationY(float degree)
	{
		float rad = degree * std::numbers::pi_v<float> / 180.0f;
		float c = std::cos(rad); float s = std::sin(rad);
		Matrix4x4 res = Identity;
		res.m[0][0] = c; res.m[0][2] = s;
		res.m[2][0] = -s; res.m[2][2] = c;
		return res;
	}
	/// <summary>
	/// Z軸周りの回転行列の作成。
	/// </summary>
	/// <param name="degree">回転角度（度数法）。</param>
	/// <returns>Z軸周りの回転を表す4x4行列。</returns>
	static Matrix4x4 RotationZ(float degree)
	{
		float rad = degree * std::numbers::pi_v<float> / 180.0f;
		float c = std::cos(rad); float s = std::sin(rad);
		Matrix4x4 res = Identity;
		res.m[0][0] = c; res.m[0][1] = -s;
		res.m[1][0] = s; res.m[1][1] = c;
		return res;
	}
	/// <summary>
	/// 3軸の合成回転行列を作成 (Z * X * Y 順)
	/// </summary>
	/// <param name="angle">各軸の回転角度(度数法)</param>
	static Matrix4x4 Rotation(const Vector3& angle)
	{
		// 数学スタイルなので、右側にある Y から先に適用される
		// つまり Z * X * Y の順番で書くと、適用順は Y -> X -> Z になる
		return RotationZ(angle.Z) * RotationX(angle.X) * RotationY(angle.Y);
	}
	/// <summary>
	/// クォータニオンから回転行列を作成
	/// </summary>
	/// <param name="q"></param>
	/// <returns></returns>
	static Matrix4x4 FromQuaternion(const Quaternion& q)
	{
		Matrix4x4 res = Identity;
		float xx = q.X * q.X; float yy = q.Y * q.Y; float zz = q.Z * q.Z;
		float xy = q.X * q.Y; float xz = q.X * q.Z; float yz = q.Y * q.Z;
		float wx = q.W * q.X; float wy = q.W * q.Y; float wz = q.W * q.Z;

		res.m[0][0] = 1.0f - 2.0f * (yy + zz);
		res.m[0][1] = 2.0f * (xy - wz);
		res.m[0][2] = 2.0f * (xz + wy);

		res.m[1][0] = 2.0f * (xy + wz);
		res.m[1][1] = 1.0f - 2.0f * (xx + zz);
		res.m[1][2] = 2.0f * (yz - wx);

		res.m[2][0] = 2.0f * (xz - wy);
		res.m[2][1] = 2.0f * (yz + wx);
		res.m[2][2] = 1.0f - 2.0f * (xx + yy);

		return res;
	}

	/* 演算子オーバーロード*/
	// DxLibの MATRIX 型への自動変換(自動で「転置（縦横変換）」して渡す)
	operator MATRIX() const 
	{
		MATRIX res;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				// 自分の[行][列] を ＤＸライブラリの[列][行]へ代入
				res.m[j][i] = m[i][j];
			}
		}
		return res;
	}
	// DxLibの MATRIX 型からの代入(自動で「転置（縦横変換）」して受け取る)
	Matrix4x4& operator=(const MATRIX& dxMat) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				// ＤＸライブラリの[行][列]を、自作の[列][行]へ代入
				this->m[j][i] = dxMat.m[i][j];
			}
		}
		return *this;
	}
	// 行列の合成（数学順序： 2 * 1）
	Matrix4x4 operator*(const Matrix4x4& r) const
	{
		Matrix4x4 res;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					res.m[i][j] += m[i][k] * r.m[k][j];
				}
			}
		}
		return res;
	}
	// 行列による座標変換（M * V）
	Vector3 operator*(const Vector3& v) const
	{
		Vector3 res;
		res.X = m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3];
		res.Y = m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3];
		res.Z = m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3];
		return res;
	}

};

/* staticメンバ変数の実装 */
inline const Matrix4x4 Matrix4x4::Identity = { {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
} };