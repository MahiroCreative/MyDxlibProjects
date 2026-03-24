#pragma once
#include "GimmickSendObject.h"
#include "StateMachine.h"

namespace MyEngine
{
	struct CollideHitInfo;
}
class GimmickLinkObject;
class FileBase;

class Button : public GimmickSendObject
{
private:
	enum class State
	{
		DOWN,
		UP,
	};

public:
	Button();
	virtual ~Button();

	void Init(const Vec3& pos, const Vec3& scale, const Quaternion& rot, std::list<Tuple<MyEngine::ColKind, MyEngine::ColliderBase*>> list, bool isGravity) override;
	void Update() override;

	virtual void OnCollideStay(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;
	virtual void OnCollideExit(MyEngine::Collidable* colider, int selfIndex, int sendIndex, const MyEngine::CollideHitInfo& hitInfo) override;

private:
	void UpdateDown();
	void UpdateUp();

	void OnDown();
	void OnUp();

private:
	StateMachine<State> m_stateMachine;

	std::shared_ptr<FileBase> m_pushSe;
	std::shared_ptr<FileBase> m_difPs;
	std::shared_ptr<FileBase> m_norPs;

	Vec3 m_startPos;

	int m_count;

	bool m_isRelease;
	bool m_isDown;
	bool m_isUp;
};

