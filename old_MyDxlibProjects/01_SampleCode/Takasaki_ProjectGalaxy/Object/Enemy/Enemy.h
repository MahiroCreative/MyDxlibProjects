#pragma once
#include "Collidable.h"
#include"Quaternion.h"

class EnemyMover;
class EnemyAI;
/*プロトタイプ宣言*/
Vec3 ToVec(Vec3 a, Vec3 b);
Vec3 norm(Vec3 a);
float lerp(float start, float end, float t);
class Enemy : public MyEngine :: Collidable
{
public:
	Enemy(int handle,Priority priority,ObjectTag tag);
	virtual ~Enemy();

	virtual void Init() = 0;
	virtual void Draw() = 0;
	virtual void Update()=0;
	//virtual int GetHP() { return m_Hp; }
protected:
	//int m_Hp;
	int m_modelHandle;
	Quaternion m_myQ;
};

