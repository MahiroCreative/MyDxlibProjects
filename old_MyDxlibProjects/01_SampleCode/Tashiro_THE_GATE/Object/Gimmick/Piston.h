#pragma once
#include "GimmickLinkObject.h"

class Piston : public GimmickLinkObject
{
public:
	Piston();
	~Piston();

	void SetMoveRange(const Vec3& start, const Vec3& end) { m_start = start; m_end = end; }

private:
	void GimmickOnUpdate() override;
	void GimmickOffUpdate() override;

	void ProcessGimmickOn() override;
	void ProcessGimmickOff() override;

private:
	Vec3 m_start;
	Vec3 m_end;

	bool m_isExpand;
	bool m_isShrink;
};

