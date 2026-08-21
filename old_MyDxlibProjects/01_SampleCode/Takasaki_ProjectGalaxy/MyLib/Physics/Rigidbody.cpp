#include "Rigidbody.h"

using namespace MyEngine;

Rigidbody::Rigidbody(const Vec3& pos)
{
	m_pos = pos;
	m_nextPos = m_pos;
	m_velocity = Vec3();
	m_dir = Vec3();
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::Init()
{
	
}

void MyEngine::Rigidbody::ChangePosition(Vec3 pos)
{
	m_pos = pos;
	m_nextPos=pos;
}

void MyEngine::Rigidbody::SetPos(Vec3 pos)
{
	m_nextPos = pos;
	m_pos = pos;
}

void Rigidbody::SetVelocity(const Vec3& velocity)
{
	m_velocity = velocity;
	// 移動している場合のみ方向を変更
	if (velocity.SqLength())
	{
		m_dir = velocity.GetNormalized();
	}
}
