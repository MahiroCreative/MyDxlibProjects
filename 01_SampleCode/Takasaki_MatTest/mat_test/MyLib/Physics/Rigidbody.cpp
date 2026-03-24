#include "Rigidbody.h"

using namespace MyEngine;

Rigidbody::Rigidbody()
{
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::Init(const Vec3& pos)
{
	m_pos = pos;
	m_nextPos = m_pos;
	m_velocity = Vec3();
	m_dir = Vec3();
}

void MyEngine::Rigidbody::ChangePosition(Vec3 pos)
{
	m_pos = pos;
	m_nextPos=pos;
}

void MyEngine::Rigidbody::SetPos(Vec3 pos)
{
	m_pos = pos;
}

void Rigidbody::SetVelocity(const Vec3& velocity)
{
	m_velocity = velocity;
	// ˆÚ“®‚µ‚Ä‚¢‚éê‡‚Ì‚İ•ûŒü‚ğ•ÏX
	if (velocity.SqLength())
	{
		m_dir = velocity.GetNormalized();
	}
}
