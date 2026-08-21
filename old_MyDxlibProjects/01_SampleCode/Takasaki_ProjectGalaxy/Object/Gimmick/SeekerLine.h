#pragma once
#include "Collidable.h"
class Player;

class SeekerLine : public MyEngine::Collidable
{
public:
	SeekerLine(std::vector<Vec3> points, int color);
	virtual ~SeekerLine();

	void Init();
	void Update();
	void Draw();
	Vec3 GetPos() { return m_rigid->GetPos(); }

	virtual void OnTriggerEnter(std::shared_ptr<Collidable> colider,MyEngine::ColliderBase::ColideTag ownTag,MyEngine::ColliderBase::ColideTag targetTag);

private:
	std::vector<Vec3> m_points;
	std::shared_ptr<Player> m_player;
	Vec3 m_playerStartPos;
	Vec3 m_velocity;


	int m_color;
	int m_hitPointNum;
	float m_speed;
	float m_num;
	int m_ratio;
	int m_length;
};