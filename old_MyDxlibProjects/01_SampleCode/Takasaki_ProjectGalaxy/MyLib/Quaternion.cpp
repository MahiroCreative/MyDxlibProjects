#include "Quaternion.h"


Quaternion AngleAxis(float angle, const Vec3& axis)
{
	Quaternion result;

	float halfRad = angle * DX_PI_F / 180 * 0.5f;
	float sin = std::sin(halfRad);
	float cos = std::cos(halfRad);
	auto normAxis = axis.GetNormalized();

	result = Quaternion(cos,normAxis.x * sin, normAxis.y * sin, normAxis.z * sin);

	return result;
}

float Dot(const Quaternion& q1, const Quaternion& q2)
{
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

