#include "SceneManager.h"
#include <DxLib.h>
#ifdef _DEBUG
#include "SceneDebug.h"
#endif
#include "SceneTitle.h"
#include "SceneOption.h"
#include "Game.h"
#include "Application.h"
#include "SoundManager.h"
#include "File.h"

namespace
{
	// オプションで使うファイルID
	const wchar_t* const FILE_WINDOW = L"I_OptionWindow";
	const wchar_t* const FILE_COMMON_FRAME = L"I_CommonFrame";
	const wchar_t* const FILE_COMMON_SELECT_FRAME = L"I_CommonSelectFrame";
	const wchar_t* const FILE_SOUND_BAR = L"I_MainSoundBar";
	const wchar_t* const FILE_VOLUME_POINT = L"I_VolumePoint";

	constexpr int FADE_FRAME = 30;
	constexpr float FADE_SPEED = 1.0f / FADE_FRAME;
}

SceneManager::SceneManager() :
	m_scene(nullptr),
	m_updateFunc(&SceneManager::NormalUpdate),
	m_drawFunc(&SceneManager::DrawNormal),
	m_bgmH(-1),
	m_fadeRate(0.0f)
{
}

SceneManager::~SceneManager()
{
}

SceneManager& SceneManager::GetInstance()
{
	static SceneManager mgr;
	return mgr;
}

void SceneManager::Init()
{
	// 最初のシーンのメモリ確保
#ifdef _DEBUG
	m_scene = std::make_shared<SceneDebug>();
#else
	m_scene = std::make_shared<SceneTitle>();
	m_fadeFunc = &SceneManager::FadeIn;
	m_fadeRate = 1.0f;
	m_isFade = true;
#endif

	auto& fileMgr = FileManager::GetInstance();
	fileMgr.Load(FILE_WINDOW, true);
	fileMgr.Load(FILE_COMMON_FRAME, true);
	fileMgr.Load(FILE_COMMON_SELECT_FRAME, true);
	fileMgr.Load(FILE_SOUND_BAR, true);
	fileMgr.Load(FILE_VOLUME_POINT, true);
	m_scene->Init();
	// 非同期ロードの開始
	m_updateFunc = &SceneManager::FileLoadingUpdate;
	m_drawFunc = &SceneManager::FileLoadingDraw;
}

void SceneManager::Update()
{
	if (m_option)
	{
		m_option->Update(false);
		return;
	}
	(this->*m_updateFunc)();
}

void SceneManager::Draw() const
{
	(this->*m_drawFunc)();
	if (m_option)
	{
		m_option->Draw();
	}
}

void SceneManager::End()
{
	m_scene->End();
}

void SceneManager::Change(const std::shared_ptr<SceneBase>& next)
{
	printf("シーンの変更\n");
	// フェード処理初期化
	m_fadeRate = 0.0f;
	// フェードアウトへ
	m_updateFunc = &SceneManager::FadeOut;
	m_drawFunc = &SceneManager::DrawFade;

	m_nextScene = next;
}

void SceneManager::ChangeBgmH(int handle)
{
	m_bgmH = handle;
}

void SceneManager::OpenOption(SceneKind openScene)
{
	// 開いていれば終了
	if (m_option) return;

	auto scene = std::make_shared<SceneOption>();
	scene->Init();
	scene->CheckOpenScene(openScene);
	m_option = scene;
}

void SceneManager::CloseOption()
{
	if (!m_option) return;

	m_option->End();
	m_option = nullptr;
}

SceneKind SceneManager::GetNowSceneKind() const
{
	return m_scene->GetNowKind();
}

void SceneManager::NormalUpdate()
{
	m_scene->Update(false);
}

void SceneManager::FadeOut()
{
	m_scene->Update(true);
	m_fadeRate += FADE_SPEED;

	// フェードアウト完了したら
	if (m_fadeRate > 1.0f)
	{
		// 終了処理
		m_scene->End();
		// シーン変更
		m_scene = m_nextScene;
		// 初期化処理(非同期)
		m_scene->AsyncInit();
		// 非同期ロードの開始
		m_updateFunc = &SceneManager::FileLoadingUpdate;
		m_drawFunc = &SceneManager::FileLoadingDraw;
		return;
	}

	// ハンドルが入っていな場合は流さない
	if (m_bgmH > -1)
	{
		auto& snd = SoundManager::GetInstance();
		snd.PlayFadeBgm(m_bgmH, 1.0f - m_fadeRate);
	}
}

void SceneManager::FileLoadingUpdate()
{
	auto num = GetASyncLoadNum();
	if (num == 0)
	{
		// 初期化処理(同期)
		m_scene->Init();
		// 割合補正
		m_fadeRate = 1.0f;
		// 通常処理へ
		m_updateFunc = &SceneManager::FadeIn;
		m_drawFunc = &SceneManager::DrawFade;
	}
}

void SceneManager::FadeIn()
{
	m_scene->Update(true);
	m_fadeRate -= FADE_SPEED;

	// フェードイン完了したら
	if (m_fadeRate < 0.0f)
	{
		// 割合補正
		m_fadeRate = 0.0f;
		// シーンの処理へ
		m_updateFunc = &SceneManager::NormalUpdate;
		m_drawFunc = &SceneManager::DrawNormal;
	}

	// ハンドルが入っていな場合は流さない
	if (m_bgmH > -1)
	{
		auto& snd = SoundManager::GetInstance();
		snd.PlayFadeBgm(m_bgmH, 1.0f - m_fadeRate);
	}
}

void SceneManager::DrawNormal() const
{
	m_scene->Draw();
}

void SceneManager::DrawFade() const
{
	m_scene->Draw();
	// フェード処理
	int alpha = static_cast<int>(Game::ALPHA_VAL * m_fadeRate);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, Game::WINDOW_W, Game::WINDOW_H, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneManager::FileLoadingDraw() const
{
	// TODO: ロード中はNowLoadingを出すようにする
	DrawString(600, 360, L"ロード中", 0xffffff);
}
