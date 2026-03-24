#pragma once
//C++標準ライブラリ
#include <cmath>
#include <cassert>


struct Vector2
{
	/*メンバ変数*/
	float X = 0.0f;
	float Y = 0.0f;

	/*staticメンバ変数*/
	static const Vector2 Zero;
	static const Vector2 Left;
	static const Vector2 Right;
	static const Vector2 Up;
	static const Vector2 Down;

	/*メンバ関数*/
	//ベクトの長さ
	float Length() const;
	//ベクトルの長さの2乗(計算が早い)
	float LengthSq() const;
	//ベクトルの正規化
	Vector2 Normalize() const;

	/*staticメンバ関数*/
	//内積
	static float Dot(const Vector2& left, const Vector2& right);

	/*演算子オーバーロード*/
	//フレンド宣言でフリー関数にアクセスを許可(外部定義できるようにする)
	//A+B
	friend Vector2 operator+(const Vector2& left, const Vector2& right);
	//A+=B
	friend Vector2& operator+=(Vector2& left, const Vector2& right);
	//A-B
	friend Vector2 operator-(const Vector2& left, const Vector2& right);
	//A-=B
	friend Vector2& operator-=(Vector2& left, const Vector2& right);
	//-A
	friend Vector2 operator-(const Vector2& vec);
	//k*A(定数)
	friend Vector2 operator*(float k, const Vector2& vec);
	//A*k(定数)
	friend Vector2 operator*(const Vector2& vec, float k);
	//A*=k(定数)
	friend Vector2& operator*=(Vector2& vec, float k);
	//A/k(定数)
	friend Vector2 operator/(const Vector2& vec, float k);
	//A/=k(定数)
	friend Vector2& operator/=(Vector2& vec, float k);

};