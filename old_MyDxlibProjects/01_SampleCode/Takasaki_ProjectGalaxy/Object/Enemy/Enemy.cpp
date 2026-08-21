#include "Enemy.h"

namespace
{
	constexpr int kMaxHp = 50;
}
/*便利関数*/
//aからbへ向かうベクトル
Vec3 ToVec(Vec3 a, Vec3 b)
{
	float x = (b.x - a.x);
	float y = (b.y - a.y);
	float z = (b.z - a.z);
	return Vec3(x, y, z);
}

Vec3 norm(Vec3 a)
{
	float num = (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
	return VGet(a.x / num, a.y / num, a.z / num);
}

float lerp(float start, float end, float t)
{
	return (1 - t) * start + t * end;
}


Enemy::Enemy(int handle, Priority priority, ObjectTag tag):Collidable(priority,tag),
	//m_Hp(50),
	m_modelHandle(handle)
{
}

Enemy::~Enemy()
{
}
