#pragma once
#include "GimmickLinkObject.h"

class FileBase;
class Door : public GimmickLinkObject
{
public:
	Door();
	~Door();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
	void Restart() override;

private:
	void GimmickOnUpdate() override;
	void GimmickOffUpdate() override;

	void ProcessGimmickOn() override;
	void ProcessGimmickOff() override;


private:
	std::shared_ptr<FileBase> m_openSe;

	Vec3 m_startPos;
	Vec3 m_right;
	
	bool m_isStart;
	bool m_isOpen;
	bool m_isClose;
};

