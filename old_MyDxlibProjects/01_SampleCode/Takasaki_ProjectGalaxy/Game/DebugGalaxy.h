#pragma once
#include "Galaxy.h"

class Camera;
class Player;
class Planet;

class DebugGalaxy : public Galaxy
{
public:
	DebugGalaxy(std::shared_ptr<Player> playerPointer);
	virtual ~DebugGalaxy();

	void Init();
	void Update();
	void Draw();

	using managerState_t = void(DebugGalaxy::*)();
	managerState_t m_managerUpdate;

	using managerState_t = void(DebugGalaxy::*)();
	managerState_t m_managerDraw;

private:
	std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<Planet>> planet;
	std::shared_ptr<Player> player;
};

