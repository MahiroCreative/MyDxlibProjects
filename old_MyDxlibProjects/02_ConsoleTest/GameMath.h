#pragma once
#include <string>
#include <math.h>
using namespace std;

struct Vector3
{
	/*メンバ変数*/
	float X = 0;
	float Y = 0;
	float Z = 0;

	/*メンバ関数*/
	/// <summary>
	/// ベクトルの長さを取得
	/// </summary>
	float Length()
	{
		//三平方の定理
		return sqrtf(X * X + Y * Y + Z * Z);
	}
	/// <summary>
	/// ベクトル成分を文字列で返す
	/// </summary>
	/// <returns>string型</returns>
	string ToString()
	{
		return to_string(X)+","+to_string(Y)+","+to_string(Z);
	}

	/*staticメンバ関数*/
	/// <summary>
	/// 内積の計算
	/// </summary>
	static float Dot(const Vector3& left, const Vector3& right)
	{
		//変数
		float ans;
		//計算
		ans = left.X * right.X + left.Y * right.Y + left.Z *right.Z;
		//return
		return ans;
	}
	/// <summary>
	/// 外積の計算
	/// </summary>
	static Vector3 Cross(const Vector3& left,const Vector3& right)
	{
		//変数
		Vector3 ans;
		//計算
		ans.X = left.Y * right.Z - left.Z * right.Y;
		ans.Y = left.Z * right.X - left.X * right.Z;
		ans.Z = left.X * right.Y - left.Y * right.X;
		//return
		return ans;
	}

	/*演算子オーバーロード*/
	//単項演算子
	Vector3 operator+(const Vector3& right)
	{
		Vector3 ans;
		ans.X = this->X + right.X;
		ans.Y = this->Y + right.Y;
		ans.Z = this->Z + right.Z;
		return ans;
	}
	Vector3 operator-(const Vector3& right)
	{
		Vector3 ans;
		ans.X = this->X - right.X;
		ans.Y = this->Y - right.Y;
		ans.Z = this->Z - right.Z;
		return ans;
	}
	Vector3 operator*(const float& right)
	{
		Vector3 ans;
		ans.X = this->X * right;
		ans.Y = this->Y * right;
		ans.Z = this->Z * right;
		return ans;
	}
	Vector3 operator/(const float& right)
	{
		Vector3 ans;
		ans.X = this->X / right;
		ans.Y = this->Y / right;
		ans.Z = this->Z / right;
		return ans;
	}
	void operator+=(const Vector3& right)
	{
		this->X += right.X;
		this->Y += right.Y;
		this->Z += right.Z;
	}
	void operator-=(const Vector3& right)
	{
		this->X -= right.X;
		this->Y -= right.Y;
		this->Z -= right.Z;
	}


};