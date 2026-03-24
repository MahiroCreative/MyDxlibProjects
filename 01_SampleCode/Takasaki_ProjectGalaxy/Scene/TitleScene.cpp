#include <DxLib.h>
#include <cassert>
#include "Application.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include"GamePlayingScene.h"
#include"Game.h"
#include"Pad.h"



namespace
{
	constexpr int kFadeFrameMax = 60;
	constexpr int kStandByFrame = 120;

	constexpr int kLightningFrameMax = 200;

	const char* kTitleGraphName = "galaxy_titleLogo_pro.png";
	const char* kFrameName = "Frame.png";
	const char* kTitleBGMName = "Title.mp3";
	const char* kGameStartSEName = "StartGame2.mp3";
	const char* kGameBGMName = "GamePlaying.mp3";

}

TitleScene::TitleScene(SceneManager& manager) :
	Scene(manager),
	m_btnFrame(0),
	m_fadeSpeed(1)
{
	m_updateFunc = &TitleScene::NormalUpdate;
	m_drawFunc = &TitleScene::NormalDraw;
}

TitleScene::~TitleScene()
{
}

void TitleScene::Load()
{
}

void TitleScene::Update()
{

	(this->*m_updateFunc)();

	Pad::Update();
}

void TitleScene::Draw()
{
	(this->*m_drawFunc)();
}

void TitleScene::FadeInUpdate()
{
	m_fps = GetFPS();

	m_frame--;
	if (m_frame <= 0)
	{
		m_updateFunc = &TitleScene::NormalUpdate;
		m_drawFunc = &TitleScene::NormalDraw;
	}
}

void TitleScene::NormalUpdate()
{
	m_fps = GetFPS();

	if (Pad::IsTrigger(PAD_INPUT_1))
	{
		m_updateFunc = &TitleScene::FadeOutUpdate;
		m_drawFunc = &TitleScene::FadeDraw;
	}

	m_btnFrame += m_fadeSpeed;
	if (m_btnFrame > kFadeFrameMax)m_fadeSpeed *= -1;
	if (m_btnFrame < 0)m_fadeSpeed *= -1;

}

void TitleScene::FadeOutUpdate()
{
	m_fps = GetFPS();
	m_frame++;
	if (m_frame >= 120) {
		m_isGamePlaying = true;
	}
}

void TitleScene::ChangeScene(std::shared_ptr<Scene> next)
{
	m_manager.ChangeScene(next);
}

void TitleScene::FadeDraw()
{
	int alpha = static_cast<int>(255 * (static_cast<float>(m_frame) / kFadeFrameMax));

	SetDrawBlendMode(DX_BLENDMODE_MULA, alpha);
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, 0x000000, true);
	DrawBox(0, 0, m_frame * 30, 1600, 0x001111, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (m_isGamePlaying)
	{
		ChangeScene(std::make_shared<GamePlayingScene>(m_manager));
	}
	DrawLine(m_frame * 30, 0, m_frame * 30, 900, 0x44ffff);
}

void TitleScene::NormalDraw()
{
	int alpha = static_cast<int>(255 * (static_cast<float>(m_btnFrame) / kFadeFrameMax));
}
