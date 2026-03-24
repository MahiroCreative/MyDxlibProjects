#include "SceneManager.h"
#include "Scene.h"

namespace NextScene
{

}

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	// ––”ö‚Ì‚İÀs
	m_scenes.back()->Update();
}

void SceneManager::Draw()
{
	// æ“ª‚©‚ç‡‚É•`‰æ(ÅŒã‚ÉÏ‚ñ‚¾‚à‚Ì‚ªÅŒã‚É•`‰æ‚³‚ê‚é)
	for (auto& scene : m_scenes)
	{
		scene->Draw();
	}
}

void SceneManager::ChangeScene(std::shared_ptr<Scene> nextScene)
{
	if (m_scenes.empty()) {	// ƒŠƒXƒg‚ª‹ó‚Á‚Û‚¾‚Á‚½‚ç“ü‚ê‘Ö‚¦‚é‚Ì‚Å‚Í‚È‚­
		m_scenes.push_back(nextScene);	// ––”ö‚É’Ç‰Á‚·‚é
	}
	else {
		m_scenes.back() = nextScene;	// ‚·‚Å‚Éˆê‚ÂˆÈã‚ ‚Á‚½‚ç––”ö‚Ì‚à‚Ì‚ğ“ü‚ê‘Ö‚¦‚é
	}
}

void SceneManager::InsertScene(std::shared_ptr<Scene> nextScene)
{
	int b = 0;
	m_scenes.insert(m_scenes.begin(), nextScene);
}



void SceneManager::PushScene(std::shared_ptr<Scene> scene)
{
	m_scenes.push_back(scene);
}

void SceneManager::PopScene()
{
	m_scenes.pop_back();
}

void SceneManager::ResetScene(std::shared_ptr<Scene> scene)
{
	m_scenes.clear();
	m_scenes.push_back(scene);
}

std::shared_ptr<Scene> SceneManager::LookBack()
{
	return m_scenes.back();
}
