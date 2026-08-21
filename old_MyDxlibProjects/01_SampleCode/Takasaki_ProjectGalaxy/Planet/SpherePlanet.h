#pragma once
#include "Planet.h"
class SpherePlanet : public Planet
{
public:
	SpherePlanet(Vec3 pos,int color,float m_gravityPower, int modelHandle);
	virtual ~SpherePlanet();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	virtual Vec3 GravityEffect(std::shared_ptr<Collidable> obj);
	virtual Vec3 GetNormVec(Vec3 pos);
	virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerExit(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	
protected:
	int m_enemyCount;
	int m_color;
	int m_modelHandle;
};

