#include "GameManager.h"
#include"Physics.h"
#include"SerialPlanetGalaxy.h"
#include"DebugGalaxy.h"
#include"Player.h"
#include"ModelManager.h"
using namespace MyEngine;

namespace
{
	const char* kPlayerFileName = "SpaceHarrier.mv1";
}

GameManager::GameManager() :
	player(std::make_shared<Player>(ModelManager::GetInstance().GetModelData(kPlayerFileName)))
{
	galaxy.push_back(std::make_shared<SerialPlanetGalaxy>(player));
}

GameManager::~GameManager()
{
}

void GameManager::Init()
{
	galaxy.back()->Init();
}

void GameManager::Update()
{
	galaxy.back()->Update();
	if (galaxy.back()->GetClear())
	{
		galaxy.pop_back();
		//galaxy.push_back();/*ここでステージ選択のフィールドを入れなおす*/
	}
	if (galaxy.size() == 0)
	{
		m_isClearFlag = true;
	}
}

void GameManager::Draw()
{
	galaxy.back()->Draw();
}

void GameManager::IntroUpdate()
{
}

void GameManager::IntroDraw()
{
}

void GameManager::GamePlayingUpdate()
{
}

void GameManager::GamePlayingDraw()
{
}
