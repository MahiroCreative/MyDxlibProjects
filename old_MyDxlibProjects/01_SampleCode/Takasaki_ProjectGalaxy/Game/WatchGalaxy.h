#pragma once
#include "Galaxy.h"
class Camera;
class Planet;

class WatchGalaxy : public Galaxy
{
public:
	WatchGalaxy(std::shared_ptr<Player> playerPointer);
	virtual ~WatchGalaxy();
	void Init();
	void Update();
	void Draw();

	using managerState_t = void(WatchGalaxy::*)();
	managerState_t m_managerUpdate;

	using managerState_t = void(WatchGalaxy::*)();
	managerState_t m_managerDraw;

private:
	std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<Planet>> planet;
	std::shared_ptr<Player> player;

	Vec3 cameraPos;
};

