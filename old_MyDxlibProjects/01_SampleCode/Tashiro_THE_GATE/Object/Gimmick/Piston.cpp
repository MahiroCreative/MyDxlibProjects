#include "Piston.h"
#include "Easing.h"

Piston::Piston() :
	GimmickLinkObject(Priority::STATIC, ObjectTag::PISTON),
	m_isExpand(false),
	m_isShrink(false)
{
}

Piston::~Piston()
{
}

void Piston::GimmickOnUpdate()
{
	if (m_isExpand) return;

	auto pos = m_rigid.GetPos();
	pos = Easing::Lerp(pos, m_end, 0.02f);
	if ((pos - m_end).SqLength() < 0.001f)
	{
		pos = m_end;
		m_isExpand = true;
	}
	m_rigid.SetPos(pos);
}

void Piston::GimmickOffUpdate()
{
	if (m_isShrink) return;

	auto pos = m_rigid.GetPos();
	pos = Easing::Lerp(pos, m_start, 0.02f);
	if ((pos - m_start).SqLength() < 0.001f)
	{
		pos = m_start;
		m_isShrink = true;
	}
	m_rigid.SetPos(pos);
}

void Piston::ProcessGimmickOn()
{
	m_isExpand = false;
}

void Piston::ProcessGimmickOff()
{
	m_isShrink = false;
}
