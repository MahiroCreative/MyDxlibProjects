#pragma once
#include "Planet.h"
class DumbbellPlanet : public Planet
{
public:
	DumbbellPlanet(Vec3 pos, int color,int modelHandle);
	virtual ~DumbbellPlanet();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	virtual Vec3 GravityEffect(std::shared_ptr<Collidable> obj);
	virtual Vec3 GetNormVec(Vec3 pos);
	// 衝突したとき
	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider, MyEngine::ColliderBase::ColideTag ownTag, MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider, MyEngine::ColliderBase::ColideTag ownTag, MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerExit(std::shared_ptr<Collidable> colider, MyEngine::ColliderBase::ColideTag ownTag, MyEngine::ColliderBase::ColideTag targetTag);

protected:
	int m_color;
	int m_modelHandle;
	Vec3 m_gravityDir;
};

