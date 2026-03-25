#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include <string>
#include <cmath>
#include <numbers> // C++20の数学定数を使用

struct Matrix4x4
{
	// DxLibに合わせて [行][列] の順（m[row][column]）
	float m[4][4] = { 0 };

	/* staticメンバ変数宣言 */
	static const Matrix4x4 Identity; // 単位行列

	/* メンバ関数 */
	// 文字列変換 
	std::string ToString() const {
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
	/// <returns></returns>
	static Matrix4x4 CreateTranslation(const Vector3& pos) {
		Matrix4x4 res = Identity;
		res.m[3][0] = pos.X; // 4行目の0..2列目が座標成分
		res.m[3][1] = pos.Y;
		res.m[3][2] = pos.Z;
		return res;
	}
	/// <summary>
	/// 拡大縮小行列の作成
	/// </summary>
	/// <param name="scale">拡大量(vector3)</param>
	/// <returns></returns>
	static Matrix4x4 CreateScaling(const Vector3& scale) {
		Matrix4x4 res = Identity;
		res.m[0][0] = scale.X;
		res.m[1][1] = scale.Y;
		res.m[2][2] = scale.Z;
		return res;
	}
	/// <summary>
	/// X軸回転行列の作成
	/// </summary>
	/// <param name="angleDegree"></param>
	/// <returns></returns>
	static Matrix4x4 CreateRotationX(float angleDegree) {
		float rad = angleDegree * std::numbers::pi_v<float> / 180.0f;
		float s = std::sin(rad);
		float c = std::cos(rad);
		Matrix4x4 res = Identity;
		res.m[1][1] = c;  res.m[1][2] = s;
		res.m[2][1] = -s; res.m[2][2] = c;
		return res;
	}
	/// <summary>
	///  Y軸回転行列の作成
	/// </summary>
	/// <param name="angleDegree"></param>
	/// <returns></returns>
	static Matrix4x4 CreateRotationY(float angleDegree) {
		float rad = angleDegree * std::numbers::pi_v<float> / 180.0f;
		float s = std::sin(rad);
		float c = std::cos(rad);
		Matrix4x4 res = Identity;
		res.m[0][0] = c;  res.m[0][2] = -s;
		res.m[2][0] = s;  res.m[2][2] = c;
		return res;
	}
	/// <summary>
	/// Z軸回転行列の作成
	/// </summary>
	/// <param name="angleDegree"></param>
	/// <returns></returns>
	static Matrix4x4 CreateRotationZ(float angleDegree) {
		float rad = angleDegree * std::numbers::pi_v<float> / 180.0f;
		float s = std::sin(rad);
		float c = std::cos(rad);
		Matrix4x4 res = Identity;
		res.m[0][0] = c;  res.m[0][1] = s;
		res.m[1][0] = -s; res.m[1][1] = c;
		return res;
	}
	/// <summary>
	/// /// オイラー角(Vector3)から合成済みの回転行列を作成する
	/// /// 回転順序は X -> Y -> Z (DxLib標準)
	/// /// </summary>
	static Matrix4x4 CreateRotation(const Vector3& rotation) {
		// 各軸の回転行列を作成
		Matrix4x4 rotX = CreateRotationX(rotation.X);
		Matrix4x4 rotY = CreateRotationY(rotation.Y);
		Matrix4x4 rotZ = CreateRotationZ(rotation.Z);

		// X → Y → Z の順で合成
		return rotX * rotY * rotZ;
	}

	/* 演算子オーバーロード*/
	// 行列同士の掛け算
	// // DxLib(行ベクトル)仕様：左の変換を適用したあと、右の変換を適用する
	Matrix4x4 operator*(const Matrix4x4& right) const {
		Matrix4x4 res;
		for (int i = 0; i < 4; ++i) { // 行
			for (int j = 0; j < 4; ++j) { // 列
				res.m[i][j] =
					m[i][0] * right.m[0][j] +
					m[i][1] * right.m[1][j] +
					m[i][2] * right.m[2][j] +
					m[i][3] * right.m[3][j];
			}
		}
		return res;
	}

	Matrix4x4& operator*=(const Matrix4x4& right) {
		*this = *this * right;
		return *this;
	}
	// DxLibの MATRIX 型への自動変換
	// これにより MV1SetMatrix(handle, myMat) と書けるようになります
	operator MATRIX() const {
		MATRIX res;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				res.m[i][j] = m[i][j];
			}
		}
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