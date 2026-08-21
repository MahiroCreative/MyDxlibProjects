#pragma once
#include"Vec3.h"
#include<memory>
#include<vector>

class Vec3;
class Galaxy;
class Player;

class GameManager
{
public:
	GameManager();
	~GameManager();
	void Init();
	void Update();
	void Draw();

	bool GetGameOver() const { return m_isGameOverFlag; }
	bool GetClear() const { return m_isClearFlag; }

	using managerState_t = void(GameManager::*)();
	managerState_t m_managerUpdate;

	using managerState_t = void(GameManager::*)();
	managerState_t m_managerDraw;
private:
	void IntroUpdate();
	void IntroDraw();

	void GamePlayingUpdate();
	void GamePlayingDraw();

private:

	std::vector<std::shared_ptr<Galaxy>> galaxy;
	std::shared_ptr<Player> player;

	//このフラグを見てシーンを移行する
	bool m_isGameOverFlag = false;
	bool m_isClearFlag = false;
};

