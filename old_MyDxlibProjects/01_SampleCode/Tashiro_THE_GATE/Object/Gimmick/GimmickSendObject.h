#pragma once
#include "Object/Object3DBase.h"

class GimmickLinkObject;

class GimmickSendObject : public Object3DBase
{
public:
	GimmickSendObject(Priority priority, ObjectTag tag);
	virtual ~GimmickSendObject();

	void SetLinkObject(GimmickLinkObject* linkObj) { m_linkObj = linkObj; }

protected:
	GimmickLinkObject* m_linkObj;
};

