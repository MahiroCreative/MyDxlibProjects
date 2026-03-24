#pragma once
#include "Collidable.h"
#include"ColliderSphere.h"

class Player;

class StarCapture : public MyEngine::Collidable
{
public:
	StarCapture(Vec3 pos,int radius=3,int captureRadius=50);
	virtual ~StarCapture();

	void Init();
	void Update();
	void Draw();

	virtual void OnCollideEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerStay(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
	virtual void OnTriggerExit(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);
private:
	using StarCaptureState_t = void(StarCapture::*)();
	StarCaptureState_t m_stateUpdate;

	void WaitingUpdate();
	void CaptureUpdate();


private:
	float m_ratio;
	int m_captureColLifeTime;
	int m_recastTime;
	int m_modelhandle;
	int m_radius;
	int m_captureRadius;
	int m_captureFrame;
	std::shared_ptr<MyEngine::ColliderSphere> m_captureCol;
	std::shared_ptr<MyEngine::Collidable> m_target;
	std::shared_ptr<Player> m_player;
	Vec3 m_playerStartPos;
	bool m_isCapturePlayer;
	bool m_isCreateCol;
};

