#pragma once
#include "Vec3.h"
#include <cmath>

class Quaternion
{
public:
	Quaternion()
	{
		Qu.w = 1;
		Qu.x = 0;
		Qu.y = 0;
		Qu.z = 0;
	};
	Quaternion(float _w, float _x, float _y, float _z)
	{
		Qu.w = _w;
		Qu.x = _x;
		Qu.y = _y;
		Qu.z = _z;
	};
	virtual ~Quaternion() {}

	struct Q
	{
		float w;
		float x;
		float y;
		float z;

		Q operator * (const Q& _q) const
		{
			Q tempQ;

			/*クオータニオンの掛け算*/
			tempQ.w = w * _q.w - x * _q.x - y * _q.y - z * _q.z;//実部
			tempQ.x = w * _q.x + x * _q.w + y * _q.z - z * _q.y;//虚部x
			tempQ.y = w * _q.y + y * _q.w + z * _q.x - x * _q.z;//虚部y
			tempQ.z = w * _q.z + z * _q.w + x * _q.y - y * _q.x;//虚部z

			return tempQ;
		}
	};

	Q Qu;
private:
	
	

public:
	Quaternion QMult(Quaternion a, Quaternion b)
	{
		Quaternion temp;

		/*クオータニオンの掛け算*/
		temp.Qu.w = a.Qu.w * b.Qu.w - a.Qu.x * b.Qu.x - a.Qu.y * b.Qu.y - a.Qu.z * b.Qu.z;//実部
		temp.Qu.x = a.Qu.w * b.Qu.x + a.Qu.x * b.Qu.w + a.Qu.y * b.Qu.z - a.Qu.z * b.Qu.y;//虚部x
		temp.Qu.y = a.Qu.w * b.Qu.y + a.Qu.y * b.Qu.w + a.Qu.z * b.Qu.x - a.Qu.x * b.Qu.z;//虚部y
		temp.Qu.z = a.Qu.w * b.Qu.z + a.Qu.z * b.Qu.w + a.Qu.x * b.Qu.y - a.Qu.y * b.Qu.x;//虚部z

		return temp;
	}
	//回転クォータニオン
	Quaternion CreateRotationQuaternion(double radian, Vec3 Axis)
	{
		Quaternion ans;
		double norm;
		double ccc, sss;

		ans.Qu.w = ans.Qu.x = ans.Qu.y = ans.Qu.z = 0.0;

		norm = Axis.x * Axis.x + Axis.y * Axis.y + Axis.z * Axis.z;
		if (norm <= 0.0) return ans;

		norm = 1.0 / sqrt(norm);
		Axis.x *=static_cast<float> (norm);
		Axis.y *= static_cast<float> (norm);
		Axis.z *= static_cast<float> (norm);

		ccc = cos(0.5 * radian);
		sss = sin(0.5 * radian);

		ans.Qu.w =static_cast<float>(ccc);
		ans.Qu.x = static_cast<float>(sss) * Axis.x;
		ans.Qu.y = static_cast<float>(sss) * Axis.y;
		ans.Qu.z = static_cast<float>(sss) * Axis.z;

		return ans;
	}
	void SetQuaternion(Vec3 pos) { Qu.w = 1.0; Qu.x = pos.x; Qu.y = pos.y; Qu.z = pos.z; }

	void SetMove(float _angle, Vec3 _axis)//回転軸と角速度の設定
	{
		_axis.Normalize();
		Qu.w = cos(_angle / 2.0f);//実部
		Qu.x = _axis.x * sin(_angle / 2.0f);
		Qu.y = _axis.y * sin(_angle / 2.0f);
		Qu.z = _axis.z * sin(_angle / 2.0f);
	}
	/// <summary>
	/// 平行移動
	/// </summary>
	/// <param name="m_pos"></param>
	/// <param name="_vec"></param>
	/// <returns></returns>
	Vec3 Move(Vec3& m_pos, Vec3& _vec)
	{
		Q qPos, qInv;
		Vec3 vPos;

		//3次元座標をクオータニオンに変換
		qPos.w = 1.0f;
		qPos.x = m_pos.x;
		qPos.y = m_pos.y;
		qPos.z = m_pos.z;

		//回転クォータニオンのインバースの作成
		//逆クォータニオンを出すのは大変なので、
		//3次元だと同じ値になる共役クオータニオンで作成(虚部だけマイナス反転)
		qInv.w = Qu.w;
		qInv.x = -Qu.x;
		qInv.y = -Qu.y;
		qInv.z = -Qu.z;

		//回転後のクオータニオンの作成
		qPos = Qu * qPos * qInv;

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

	Vec3 ToVec3()
	{
		return Vec3(Qu.x, Qu.y, Qu.z);
	}

	MATRIX ToMat()
	{
		MATRIX matQ;

		float x2 = Qu.x * Qu.x;
		float y2 = Qu.y * Qu.y;
		float z2 = Qu.z * Qu.z;
		float w2 = Qu.w * Qu.w;

		float r00 = x2 - y2 - z2 + w2;
		float r01 = 2.0f * (Qu.x * Qu.y + Qu.z * Qu.w);
		float r02 = 2.0f * (Qu.x * Qu.z - Qu.y * Qu.w);

		float r10 = 2.0f * (Qu.x * Qu.y - Qu.z * Qu.w);
		float r11 = -x2 + y2 - z2 + w2;
		float r12 = 2.0f * (Qu.y * Qu.z + Qu.x * Qu.w);

		float r20 = 2.0f * (Qu.x * Qu.z + Qu.y * Qu.w);
		float r21 = 2.0f * (Qu.y * Qu.z - Qu.x * Qu.w);
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
		Q q;
		cross = cross.GetNormalized();
		theta = theta / 2;
		Qu.x = cross.x * sin(theta);
		Qu.y = cross.y * sin(theta);
		Qu.z = cross.z * sin(theta);
		Qu.w = cos(theta);
	}

};

