#pragma once
#include "EnemySphere.h"


class Enemy;

class Killer : public EnemySphere
{
public:
	Killer(MyEngine::Collidable::Priority priority, ObjectTag tag, std::shared_ptr<MyEngine::Collidable>enemy,std::shared_ptr<MyEngine::Collidable>target, Vec3 pos, Vec3 velocity, int moveNum, int color);
	~Killer();
	virtual void Init();
	virtual void Update();
	virtual void Draw();

	virtual void Hit();

	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual bool IsDelete() { return m_isDeleteFlag; }
	void SetCounterFlag() { m_counterFlag = true; }
	Vec3 GetVelocity() { return m_velocity; }
	void SetVelocity(Vec3 vel) { m_velocity = vel; }
	bool GetCounterFlag() { return m_counterFlag; }
	//メンバ関数ポインタ
	using MoveState_t = void(Killer::*)();
	MoveState_t m_moveUpdate;

protected:
	virtual void SearchUpDate();

	virtual void  StraightUpdate();//球を直線状に飛ばす

	virtual void CounterUpdate();
private:
	std::shared_ptr<MyEngine::Collidable> m_target;
	int m_pointeLightHandle;
};

