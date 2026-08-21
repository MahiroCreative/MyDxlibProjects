#pragma once
#include "Object/Object3DBase.h"
#include "StateMachine.h"

class GimmickLinkObject : public Object3DBase
{
protected:
	enum class State
	{
		GIMMICK_ON,
		GIMMICK_OFF,
	};

public:
	GimmickLinkObject(Priority priority, ObjectTag tag);
	virtual ~GimmickLinkObject();

	virtual void Restart() override;
	virtual void Update() override;

	virtual void OnGimmick();
	virtual void StopGimmick();

protected:
	virtual void GimmickOnUpdate() abstract;
	virtual void GimmickOffUpdate() abstract;

	virtual void ProcessGimmickOn() {}
	virtual void ProcessGimmickOff() {}

protected:
	StateMachine<State> m_stateMachine;

private:
	bool m_isOn;
	bool m_isRelease;
	int m_count;
};

