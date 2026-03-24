#pragma once
#include "MyMath.h"

//namespace.
using namespace std;

//実装
namespace MyMath
{
	/// <summary>
	/// 3次元ベクトルの構造体(float)
	/// </summary>
	struct Vector3
	{
		/*メンバ変数*/
		float X = 0, Y = 0, Z = 0;

		/*演算子オーバーロード*/
		//単項演算子(const付けるとメンバ変数の変更を行えなくなる)
		Vector3 operator+(const Vector3& right) const
		{
			Vector3 ans;
			ans.X = this->X + right.X;
			ans.Y = this->Y + right.Y;
			ans.Z = this->Z + right.Z;
			return ans;
		}
		Vector3 operator-(const Vector3& right) const
		{
			Vector3 ans;
			ans.X = this->X - right.X;
			ans.Y = this->Y - right.Y;
			ans.Z = this->Z - right.Z;
			return ans;
		}
		Vector3 operator*(const float& right) const
		{
			Vector3 ans;
			ans.X = this->X * right;
			ans.Y = this->Y * right;
			ans.Z = this->Z * right;
			return ans;
		}
		Vector3 operator/(const float& right) const
		{
			assert(right != 0.0f && "0で除算しています！");
			//実処理
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
		bool operator==(const Vector3& right) const
		{
			bool isX = X == right.X;
			bool isY = Y == right.Y;
			bool isZ = Z == right.Z;
			return isX && isY && isZ;
		}
		bool operator!=(const Vector3& right) const
		{
			bool isX = X != right.X;
			bool isY = Y != right.Y;
			bool isZ = Z != right.Z;
			return isX || isY || isZ;
		}

		/*メンバ関数*/
		/// <summary>
		/// ベクトル成分の一括代入
		/// </summary>
		void Set(float x, float y, float z)
		{
			this->X = x;
			this->Y = y;
			this->Z = z;
		}
		/// <summary>
		/// ベクトルの大きさを取得
		/// </summary>
		float Magnitude() const
		{
			//平方和の平方根(2乗して足した後にルートする)
			return hypot(this->X, this->Y, this->Z);
		}
		/// <summary>
		/// ベクトルの大きさの2乗を取得(ルートしてないので早い)
		/// </summary>
		float sqrtMagnitude() const
		{
			return this->X * this->X + this->Y * this->Y + this->Z * this->Z;
		}
		/// <summary>
		/// 正規化したベクトルを取得
		/// </summary>
		Vector3 Normalize() const
		{
			Vector3 ans;
			//正規化
			ans = (*this) / this->Magnitude();
			return ans;
		}
		/// <summary>
		/// 全ての成分の絶対値を取ったベクトルを取得
		/// </summary>
		Vector3 Abs()
		{
			Vector3 ans;
			//絶対値の計算
			ans.X = abs(this->X);
			ans.Y = abs(this->Y);
			ans.Z = abs(this->Z);
			return ans;
		}
		/// <summary>
		/// ベクトル成分を文字列で返す
		/// </summary>
		string ToString() const
		{
			return to_string(X) + ":" + to_string(Y) + ":" + to_string(Z);
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
			ans = left.X * right.X + left.Y * right.Y + left.Z * right.Z;
			//return
			return ans;
		}
		/// <summary>
		/// 外積の計算
		/// </summary>
		static Vector3 Cross(const Vector3& left, const Vector3& right)
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

		/*staticメンバ関数(定数)*/
		/// <summary>
		/// 右ベクトル
		/// </summary>
		/// <returns>(1, 0, 0)</returns>
		static constexpr Vector3 Right()
		{
			return Vector3{ 1.0f,0.0f,0.0f };
		}
		/// <summary>
		/// 左ベクトル
		/// </summary>
		/// <returns>(-1, 0, 0)</returns>
		static constexpr Vector3 Left()
		{
			return Vector3{ -1.0f,0.0f,0.0f };
		}
		/// <summary>
		/// 上ベクトル
		/// </summary>
		/// <returns>(0, 1, 0)</returns>
		static constexpr Vector3 Up()
		{
			return Vector3{ 0.0f,1.0f,0.0f };
		}
		/// <summary>
		/// 下ベクトル
		/// </summary>
		/// <returns>(0, -1, 0)</returns>
		static constexpr Vector3 Down()
		{
			return Vector3{ 0.0f,-1.0f,0.0f };
		}
		/// <summary>
		/// 正面ベクトル
		/// </summary>
		/// <returns>(0, 0, 1)</returns>
		static constexpr Vector3 Forward()
		{
			return Vector3{ 0.0f,0.0f,1.0f };
		}
		/// <summary>
		/// 背面ベクトル
		/// </summary>
		/// <returns>(0, 0, -1)</returns>
		static constexpr Vector3 Back()
		{
			return Vector3{ 0.0f,0.0f,-1.0f };
		}
		/// <summary>
		/// ゼロベクトル
		/// </summary>
		/// <returns></returns>
		static constexpr Vector3 Zero()
		{
			return Vector3{ 0.0f,0.0f,0.0f };
		}
	};
}




