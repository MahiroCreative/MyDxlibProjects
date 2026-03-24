#include "Quaternion.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include "Common/Game.h"

Quaternion::Quaternion() :
    x(0.0f),
    y(0.0f),
    z(0.0f),
    w(1.0f)
{
}

Quaternion::Quaternion(float inX, float inY, float inZ, float inW) :
    x(inX),
    y(inY),
    z(inZ),
    w(inW)
{
}

Quaternion Quaternion::Conjugated() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
    Quaternion res;

    res.x = x * q.w + w * q.x - z * q.y + y * q.z;
    res.y = y * q.w + z * q.x + w * q.y - x * q.z;
    res.z = z * q.w - y * q.x + x * q.y + w * q.z;
    res.w = w * q.w - x * q.x - y * q.y - z * q.z;

    return res;
}

Vec3 Quaternion::operator*(const Vec3& vec) const
{
    Quaternion posQ = Quaternion(vec.x, vec.y, vec.z, 1.0f);

    Quaternion newPos = *this * posQ * this->Conjugated();
    
    return Vec3(newPos.x, newPos.y, newPos.z);
}

Vec3 Quaternion::GetRadian() const
{
    float q00 = w * w;
    float q11 = x * x;
    float q22 = y * y;
    float q33 = z * z;
    float q01 = w * x;
    float q02 = w * y;
    float q03 = w * z;
    float q12 = x * y;
    float q13 = x * z;
    float q23 = y * z;

    Vec3 res;

    res.x = std::atan2f(2.0f * (q23 + q01), (q00 - q11 - q22 + q33));
    res.y = -std::asinf(2.0f * (q13 - q02));
    res.z = std::atan2f(2.0f * (q12 + q03), (q00 + q11 - q22 - q33));

    return res;
}

Quaternion AngleAxis(float angle, const Vec3& axis)
{
    Quaternion result;

    float halfRad = angle * Game::DEG_2_RAD * 0.5f;
    float sin = std::sin(halfRad);
    float cos = std::cos(halfRad);
    auto normAxis = axis.GetNormalized();
    assert(normAxis.SqLength() > 0.0f);

    result = Quaternion(normAxis.x * sin, normAxis.y * sin, normAxis.z * sin, cos);

    return result;
}

float Dot(const Quaternion& q1, const Quaternion& q2)
{
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}
