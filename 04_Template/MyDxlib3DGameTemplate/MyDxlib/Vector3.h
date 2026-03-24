#pragma once
#include <cmath>
#include <cassert>
#include <string>
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
        float len = Length();
        assert(len != 0 && "Vector3::Normalize : 0 division");
        if (len <= 0.0f) return Zero;
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
	//DXlibのVECTOR型から自動変換
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