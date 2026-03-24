#pragma once
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Vec3.h"

struct Quaternion;

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4
{
public:
	float m[4][4];
	// 行->列

	Matrix4x4();

	/* 演算 */
	Matrix4x4 operator+ (const Matrix4x4& mat) const;
	Matrix4x4 operator- (const Matrix4x4& mat) const;
	Matrix4x4 operator* (const Matrix4x4& mat) const;
	Matrix4x4 operator/ (float div) const;

	/// <summary>
	/// 全値を0にする
	/// </summary>
	void Zero();
	/// <summary>
	/// 単位行列にする
	/// </summary>
	void Identity();
	/// <summary>
	/// 転置行列を返す
	/// </summary>
	/// <returns>転置行列</returns>
	Matrix4x4 Transpose();
	/// <summary>
	/// 逆行列を返す
	/// </summary>
	/// <param name="isCorrect">true:正確な結果を返すが計算が重い /false:簡易的な結果を返すが計算が軽い</param>
	/// <returns>逆行列</returns>
	Matrix4x4 Inverse(bool isCorrect = false);

	/// <summary>
	/// 座標を返す
	/// </summary>
	/// <returns>座標</returns>
	Vec3 GetPos() const;
	/// <summary>
	/// 回転行列をクオータニオンにして返す
	/// </summary>
	/// <returns>回転</returns>
	Quaternion GetRot() const;

	/// <summary>
	/// DxLib用の行列を返す
	/// </summary>
	/// <returns>DxLib::MATRIX</returns>
	MATRIX GetMATRIX() const;

private:
	/// <summary>
	/// 左辺の列と右辺の行を計算する
	/// </summary>
	/// <param name="mat">右辺側の行列</param>
	/// <param name="line">列</param>
	/// <param name="row">行</param>
	/// <returns>計算した値</returns>
	float Dot(const Matrix4x4& mat, int line, int row) const;
};

/// <summary>
/// ベクトルをスケールにした行列を返す
/// </summary>
/// <param name="size">スケール</param>
/// <returns>スケールに値が入った行列</returns>
Matrix4x4 Scale(const Vec3& size);
/// <summary>
/// ベクトルを座標にした行列を返す
/// </summary>
/// <param name="pos">座標</param>
/// <returns>座標が入った行列</returns>
Matrix4x4 Pos(const Vec3& pos);
/// <summary>
/// クオータニオンを回転行列にして返す
/// </summary>
/// <param name="q">クオータニオン(回転)</param>
/// <returns>回転行列</returns>
Matrix4x4 Rot(const Quaternion& q);
