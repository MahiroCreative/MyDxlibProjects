#pragma once
#include "Collidable.h"
class SphereBase : public MyEngine::Collidable
{
public:
	virtual bool IsDelete() { return m_isDeleteFlag; }
	void DeleteFlag() { m_isDeleteFlag = true; }
	bool GetCounterFlag() { return m_counterFlag; }

	void SetCounterFlag() { m_counterFlag = true; }
	Vec3 GetVelocity() { return m_velocity; }
	void SetVelocity(Vec3 vel) { m_velocity = vel; }
protected:
	SphereBase(MyEngine::Collidable::Priority priority, ObjectTag tag,Vec3 pos, Vec3 velocity,int color,float radius);
	virtual ~SphereBase();

	virtual void Init()=0;
	virtual void Update()=0;
	virtual void Draw()=0;

	
	
	
	
protected:
	int m_color = 0;

	bool m_isDeleteFlag = 0;
	bool m_counterFlag;

	float m_radius = 0;

	Vec3 m_velocity;
};

