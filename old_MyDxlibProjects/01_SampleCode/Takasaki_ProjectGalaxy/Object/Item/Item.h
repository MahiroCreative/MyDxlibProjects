#pragma once
#include "Collidable.h"
#include"ColliderSphere.h"

using namespace MyEngine;

class Item : public MyEngine::Collidable
{
public:
	Item(Vec3 pos, bool antiGravity=false);
	virtual ~Item();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	bool GetDeleteFlag() { return m_deleteFlag; }
	Vec3 GetUpVec() { return m_upVec; }
	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
protected:

	std::shared_ptr<ColliderSphere> m_col;
	float angle=0;
	bool m_deleteFlag = false;
	Vec3 m_nowPlanetPos;
	Vec3 m_upVec;
};

