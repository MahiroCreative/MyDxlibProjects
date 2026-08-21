#pragma once
#include "MyLib/Vec3.h"
#include <cmath>

class Quaternion
{
public:
	Quaternion()
	{
		w = 1;
		x = 0;
		y = 0;
		z = 0;
	};
	Quaternion(float _w, float _x, float _y, float _z)
	{
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	};
	Quaternion Conjugated() const
	{
		return Quaternion(w,-x, -y, -z);
	}

	Quaternion operator*(const Quaternion& q) const
	{
		Quaternion res;

		res.x = x * q.w + w * q.x - z * q.y + y * q.z;
		res.y = y * q.w + z * q.x + w * q.y - x * q.z;
		res.z = z * q.w - y * q.x + x * q.y + w * q.z;
		res.w = w * q.w - x * q.x - y * q.y - z * q.z;

		return res;
	}
	Vec3 operator*(const Vec3& vec) const
	{
		Quaternion posQ = Quaternion(1.0f,vec.x, vec.y, vec.z);

		Quaternion newPos = *this * posQ * this->Conjugated();

		return Vec3(newPos.x, newPos.y, newPos.z);
	}

	virtual ~Quaternion() {}
	float w;
	float x;
	float y;
	float z;

private:

	
public:
	Quaternion QMult(Quaternion a, Quaternion b)
	{
		Quaternion temp;

		/*クオータニオンの掛け算*/
		temp.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;//実部
		temp.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;//虚部x
		temp.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;//虚部y
		temp.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;//虚部z

		return temp;
	}
	
	//回転クォータニオン
	Quaternion CreateRotationQuaternion(double radian, Vec3 Axis)
	{
		Quaternion ans;
		double norm;
		ans.w = 1.0;
		ans.x = ans.y = ans.z = 0.0;

		norm = Axis.x * Axis.x + Axis.y * Axis.y + Axis.z * Axis.z;
		if (norm <= 0.0) return ans;

		norm = 1.0 / sqrt(norm);
		Axis.x *=static_cast<float> (norm);
		Axis.y *= static_cast<float> (norm);
		Axis.z *= static_cast<float> (norm);

		float halfRadian = 0.5f * radian;
		float ccc = cos(halfRadian);
		float sss = sin(halfRadian);

		ans.w = static_cast<float>(ccc);
		ans.x = static_cast<float>(sss) * Axis.x;
		ans.y = static_cast<float>(sss) * Axis.y;
		ans.z = static_cast<float>(sss) * Axis.z;

		return ans;
	}
	void SetQuaternion(Vec3 pos) { w = 1.0; x = pos.x; y = pos.y; z = pos.z; }

	void SetMove(float _angle, Vec3 _axis)//回転軸と角速度の設定
	{
		_axis.Normalize();
		w = cos(_angle / 2.0f);//実部
		x = _axis.x * sin(_angle / 2.0f);
		y = _axis.y * sin(_angle / 2.0f);
		z = _axis.z * sin(_angle / 2.0f);
	}
	
	Vec3 ToVec3()
	{
		return Vec3(x, y, z);
	}

	MATRIX ToMat()
	{
		MATRIX matQ;

		float x2 = x * x;
		float y2 = y * y;
		float z2 = z * z;
		float w2 = w * w;

		float r00 = x2 - y2 - z2 + w2;
		float r01 = 2.0f * (x * y + z * w);
		float r02 = 2.0f * (x * z - y * w);

		float r10 = 2.0f * (x * y - z * w);
		float r11 = -x2 + y2 - z2 + w2;
		float r12 = 2.0f * (y * z + x * w);

		float r20 = 2.0f * (x * z + y * w);
		float r21 = 2.0f * (y * z - x * w);
		float r22 = -x2 - y2 + z2 + w2;

		matQ.m[0][0] = r00;
		matQ.m[0][1] = r01;
		matQ.m[0][2] = r02;
		matQ.m[1][0] = r10;
		matQ.m[1][1] = r11;
		matQ.m[1][2] = r12;
		matQ.m[2][0] = r20;
		matQ.m[2][1] = r21;
		matQ.m[2][2] = r22;

		matQ.m[3][3] = 1.0f;

		return matQ;
	}
	
	void LookAt(Vec3 Dir)
	{
		Vec3 norm = Vec3(0, 0, 1);
		float dot = Dot(norm, Dir);
		float theta = acos(dot);
		Vec3 cross =Cross(norm, Dir);

		cross = cross.GetNormalized();
		theta = theta / 2;
		x = cross.x * sin(theta);
		y = cross.y * sin(theta);
		z = cross.z * sin(theta);
		w = cos(theta);
	}


	Vec3 Move(Vec3& m_pos, Vec3& _vec)
	{
		Quaternion qPos, qInv,Qu;
		Vec3 vPos;


		//3次元座標をクオータニオンに変換
		qPos.w = 1.0f;
		qPos.x = m_pos.x;
		qPos.y = m_pos.y;
		qPos.z = m_pos.z;

		//回転クォータニオンのインバースの作成
		//逆クォータニオンを出すのは大変なので、
		//3次元だと同じ値になる共役クオータニオンで作成(虚部だけマイナス反転)
		qInv.w = w;
		qInv.x = -x;
		qInv.y = -y;
		qInv.z = -z;

		//回転後のクオータニオンの作成
		qPos = *this * qPos * qInv;

		//３次元座標に戻す
		vPos.x = qPos.x;
		vPos.y = qPos.y;
		vPos.z = qPos.z;

		// 回転後に移動
		vPos.x += _vec.x;
		vPos.y += _vec.y;
		vPos.z += _vec.z;

		return vPos;
	}
};

Quaternion AngleAxis(float angle, const Vec3& axis);

float Dot(const Quaternion& q1, const Quaternion& q2);

