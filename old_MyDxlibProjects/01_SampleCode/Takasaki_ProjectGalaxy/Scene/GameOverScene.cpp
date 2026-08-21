#include"GameOverScene.h"
#include"Vec3.h"
#include"Pad.h"

GameOverScene::GameOverScene(SceneManager& mgr) :
	Scene(mgr)
{
	Vec3 centerPos = Vec3(800, 450, 0);

}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::Load()
{
}

void GameOverScene::Update()
{
	(this->*m_updateFunc)();

	Pad::Update();
}

void GameOverScene::Draw()
{
	(this->*m_drawFunc)();
}

void GameOverScene::FadeInUpdate()
{
	m_frame--;

	if (m_frame <= 0)
	{
		m_updateFunc = &GameOverScene::NormalUpdate;
		m_drawFunc = &GameOverScene::NormalDraw;
	}
}

void GameOverScene::NormalUpdate()
{
	
}

void GameOverScene::FadeOutUpdate()
{

}

void GameOverScene::ChangeScene(std::shared_ptr<Scene> nextScene)
{
}

void GameOverScene::FadeDraw()
{
	
}

void GameOverScene::FadeOutDraw()
{
	
}

void GameOverScene::NormalDraw()
{
	
}
