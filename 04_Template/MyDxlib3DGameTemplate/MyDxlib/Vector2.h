#pragma once
//C++標準ライブラリ
#include <cmath>
#include <cassert>
#include <string>
#include "Vector3.h"

struct Vector2
{
	/*メンバ変数*/
	float X = 0.0f;
	float Y = 0.0f;

	/*staticメンバ変数宣言(実装は外で行う)*/
	static const Vector2 Zero;
	static const Vector2 Left;
	static const Vector2 Right;
	static const Vector2 Up;
	static const Vector2 Down;

	/*メンバ関数*/
	//ベクトルの長さの2乗(計算が早い)
	float LengthSq() const
	{
		return X * X + Y * Y;
	}
	//ベクトの長さ
	float Length() const
	{
		return sqrtf(LengthSq());
	}
	//ベクトルの正規化
	Vector2 Normalize() const
	{
		//長さ出す
		float len = Length();
		//エラー検知(0除算)
		assert(len != 0 && "0 division");
		//リリース時クラッシュ回避
		if (len <= 0.0f) { return Zero; }//0以下ならば、0ベクトルを返す
		//長さを成分で割って、正規化した値を返す
		return Vector2{ X / len, Y / len };
	}
	//絶対値
	Vector2 Abs() const
	{
		return Vector2{ std::abs(X),std::abs(Y)};
	}
	//文字列変換
	std::string ToString() const
	{
		return std::to_string(X) + ":" + std::to_string(Y);
	}
	//Vector2をVector3型に変換
	Vector3 ToVector3(float z = 0.0f) const
	{
		return { X,Y,z };
	}

	/*staticメンバ関数*/
	//内積
	static float Dot(const Vector2& left, const Vector2& right)
	{
		return left.X * right.X + left.Y * right.Y;
	}
	//距離
	static float Distance(const Vector2& from, const Vector2& to)
	{
		return (to - from).Length();
	}

	/*演算子オーバーロード*/
	//単項演算子(const付けるとメンバ変数の変更を行えなくなる)
	//A+B
	Vector2 operator+(const Vector2& right) const
	{
		return Vector2{ this->X + right.X, this->Y + right.Y };
	}
	//A-B
	Vector2 operator-(const Vector2& right) const
	{
		return Vector2{ this->X - right.X, this->Y - right.Y };
	}
	//A*k(定数)
	Vector2 operator*(float k) const
	{
		return Vector2{ this->X * k, this->Y * k };
	}
	//A/k(定数)
	Vector2 operator/(float k) const
	{
		assert(k != 0 && "0 division");//0除算
		return Vector2{ this->X / k, this->Y / k };
	}
	//-A(単項マイナス)
	Vector2 operator-() const
	{
		return Vector2{ -this->X, -this->Y };
	}
	//複合代入演算子
	//A+=B
	Vector2& operator+=(const Vector2& right)
	{
		this->X += right.X;
		this->Y += right.Y;
		return *this;
	}
	//A-=B
	Vector2& operator-=(const Vector2& right)
	{
		this->X -= right.X;
		this->Y -= right.Y;
		return *this;
	}
	//A*=k(定数)
	Vector2& operator*=(float k) 
	{
		this->X *= k;
		this->Y *= k;
		return *this;
	}
	//friend関数(非メンバ関数)として定義することで、左辺が定数の演算も可能になる
	//k*A(定数*ベクトル)
	friend Vector2 operator*(float k, const Vector2& v)
	{
		return Vector2{ v.X * k, v.Y * k };
	}
};

/*staticメンバ変数の実装*/
inline const Vector2 Vector2::Zero = { 0.0f, 0.0f };
inline const Vector2 Vector2::Left = { -1.0f, 0.0f };
inline const Vector2 Vector2::Right = { 1.0f, 0.0f };
inline const Vector2 Vector2::Up = { 0.0f, -1.0f };//Dxlibは上がマイナス、下がプラス
inline const Vector2 Vector2::Down = { 0.0f, 1.0f };//Dxlibは上がマイナス、下がプラス