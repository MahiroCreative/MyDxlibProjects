#pragma once
#include "SphereBase.h"
class Player;

class PlayerSphere : public SphereBase
{
public:
	PlayerSphere(MyEngine::Collidable::Priority priority, ObjectTag tag,Vec3 pos, Vec3 velocity, int moveNum, int color= 0xff0000);
	virtual ~PlayerSphere();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	virtual void Hit();

	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider);

	//メンバ関数ポインタ
	using MoveState_t = void(PlayerSphere::*)();
	MoveState_t m_moveUpdate;

protected:
	virtual void  StraightUpdate();//球を直線状に飛ばす

protected:
};

