#include "SceneOption.h"
#include <DxLib.h>
#include <cassert>
#include <algorithm>
#include "CursorUtility.h"
#include "UIUtility.h"
#include "Game.h"
#include "File.h"
#include "SceneManager.h"
#include "SceneStageSelect.h"
#include "SoundManager.h"

namespace
{
	// 選択種類
	enum class SelectTitleCurrent : int
	{
		BGM,
		SE,
		CLOSE,
		MAX,
	};
	enum class SelectMainCurrent : int
	{
		BGM,
		SE,
		RESTART,
		RETURN_STAGE_SELECT,
		MAX
	};
	// 確認種類
	enum class CheckMainCurrent : int
	{
		CANCEL,
		RETURN,
		MAX
	};
	// 音変更
	enum class SoundChange
	{
		BGM,
		SE,
	};

	// ファイルID
	const wchar_t* const FILE_WINDOW				= L"I_OptionWindow";
	const wchar_t* const FILE_COMMON_FRAME			= L"I_CommonFrame";
	const wchar_t* const FILE_COMMON_SELECT_FRAME	= L"I_CommonSelectFrame";
	const wchar_t* const FILE_SOUND_BAR				= L"I_MainSoundBar";
	const wchar_t* const FILE_VOLUME_POINT			= L"I_VolumePoint";

	// フレーム描画位置
	constexpr int DRAW_FRAME_X = 300;
	constexpr int DRAW_FRAME_Y = 225;
	// フレーム描画間隔
	constexpr int DRAW_FRAME_Y_INTERVAL = 80;
	// フレーム画像サイズ
	constexpr float FILE_SIZE_FRAME = 0.75f;
	// サウンドバー描画位置
	constexpr int DRAW_SOUND_X = 725;
	constexpr int DRAW_SOUND_Y = DRAW_FRAME_Y;
	// サウンドバー描画間隔
	constexpr int DRAW_SOUND_Y_INTERVAL = DRAW_FRAME_Y_INTERVAL;
	// サウンドポイント描画位置
	constexpr int DRAW_SOUND_POINT_X = 454;
	// サウンドポイント描画間隔
	constexpr int DRAW_SOUND_POINT_X_INTERVAL = 542;
	// フォントサイズ
	constexpr int FONT_SIZE = 32;
	// 選択文字カラー
	constexpr unsigned int COLOR_SELECT = 0x000000;
	// 未選択文字カラー
	constexpr unsigned int COLOR_NOT_SELECT = 0xffffff;
	// タイトルから来た時の文字列
	const std::vector<std::wstring> STR_TITLE =
	{
		L"ＢＧＭ",
		L"ＳＥ",
		L"閉じる"
	};
	// メインからの来た時の文字列
	const std::vector<std::wstring> STR_MAIN =
	{
		L"ＢＧＭ",
		L"ＳＥ",
		L"再開",
		L"ステージ選択へ",
	};
	// フレーム描画位置
	constexpr int DRAW_SOUND_TXT_FRAME_X = 1065;
	constexpr int DRAW_SOUND_TXT_FRAME_SUB_Y = 0;
	// フレーム画像サイズ
	constexpr float FILE_SIZE_SOUND_TXT_FRAME_W = 0.25f;
	constexpr float FILE_SIZE_SOUND_TXT_FRAME_H = 0.75f;
	// 文字描画位置
	constexpr int DRAW_SOUND_TXT_X = DRAW_SOUND_TXT_FRAME_X - 16;
	constexpr int DRAW_SOUND_TXT_SUB_Y = 18;
	// 文字フォントサイズ
	constexpr int FONT_SIZE_SOUND = 36;
	// 文字カラー
	constexpr unsigned int COLOR_SOUND = 0xffffff;

	// 変更音量
	constexpr int CHANGE_VOLUME = 1;
	// サウンド更新を行う番号のマックス値
	constexpr int SOUND_CHECK_CURRENT_MAX = 2;
	// 文字のウェーブサイズ
	constexpr int WAVE_SIZE = 16;
	// ウェーブスピード
	constexpr float WAVE_SPEED = 3.0f;
	// リピート間隔
	constexpr int REPEATE_INTERVAL = 1;
	// スタートリピート間隔
	constexpr int START_REPEATE_INTERVAL = 10;
	// フェードスピード
	constexpr float FADE_SPEED = 0.1f;
	// フェード時の画面外の値
	constexpr int DRAW_OUT_VAL = 360;
}

SceneOption::SceneOption() :
	SceneBase(SceneKind::OPTION),
	m_updateFunc(&SceneOption::FadeInUpdate),
	m_drawFunc(&SceneOption::DrawFade),
	m_openScene(SceneKind::TITLE),
	m_selectCurrent(0),
	m_checkCurrent(0),
	m_waveCount(0),
	m_fadeCount(0.0f)
{
}

void SceneOption::AsyncInit()
{
}

void SceneOption::Init()
{
	// ファイルロード
	auto& fileMgr = FileManager::GetInstance();
	m_files[FILE_WINDOW] = fileMgr.Load(FILE_WINDOW);
	m_files[FILE_COMMON_FRAME] = fileMgr.Load(FILE_COMMON_FRAME);
	m_files[FILE_COMMON_SELECT_FRAME] = fileMgr.Load(FILE_COMMON_SELECT_FRAME);
	m_files[FILE_SOUND_BAR] = fileMgr.Load(FILE_SOUND_BAR);
	m_files[FILE_VOLUME_POINT] = fileMgr.Load(FILE_VOLUME_POINT);
	// 文字列登録
	m_selectStrList[SceneKind::TITLE] = STR_TITLE;
	m_selectStrList[SceneKind::MAIN] = STR_MAIN;
}

void SceneOption::End()
{
}

void SceneOption::Update(bool isFade)
{
	m_waveCount++;
	if (isFade) return;
	(this->*m_updateFunc)();
}

void SceneOption::Draw() const
{
	(this->*m_drawFunc)();
	
}

void SceneOption::CheckOpenScene(SceneKind openScene)
{
	m_openScene = openScene;
	if (openScene == SceneKind::TITLE)
	{
		m_uiList = UIMoveData::Make(static_cast<int>(SelectTitleCurrent::MAX), DRAW_FRAME_X, DRAW_FRAME_Y, 0, DRAW_FRAME_Y_INTERVAL);
	}
	else if (openScene == SceneKind::MAIN)
	{
		m_uiList = UIMoveData::Make(static_cast<int>(SelectMainCurrent::MAX), DRAW_FRAME_X, DRAW_FRAME_Y, 0, DRAW_FRAME_Y_INTERVAL);
	}
	else
	{
		assert(false && "開くシーンがおかしいです");
	}
}

void SceneOption::FadeInUpdate()
{
	// フェード更新
	m_fadeCount += FADE_SPEED;
	// フェード中は処理終了
	if (m_fadeCount < 1.0f) return;
	
	m_fadeCount = 1.0f;
	m_drawFunc = &SceneOption::DrawCommon;
	// タイトルの時のアップデート
	if (m_openScene == SceneKind::TITLE)
	{
		m_updateFunc = &SceneOption::SelectTitleUpdate;
	}
	// メインの時のアップデート
	else if (m_openScene == SceneKind::MAIN)
	{
		m_updateFunc = &SceneOption::SelectMainUpdate;
	}
}

void SceneOption::FadeOutUpdate()
{
	// フェード行進
	m_fadeCount -= FADE_SPEED;
	// フェード終了したらオプションを閉じる
	if (m_fadeCount < 0) m_scnMgr.CloseOption();
}

void SceneOption::SelectTitleUpdate()
{
	if (CursorUtility::CursorUp<SelectTitleCurrent>(m_selectCurrent, SelectTitleCurrent::MAX)) m_waveCount = 0;
	if (CursorUtility::CursorDown<SelectTitleCurrent>(m_selectCurrent, SelectTitleCurrent::MAX)) m_waveCount = 0;
	// BGM・SE調整
	if (m_selectCurrent < SOUND_CHECK_CURRENT_MAX)
	{
		SoundChangeUpdate(m_selectCurrent);
	}

	auto& input = Input::GetInstance();
	bool isClose = false;
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		// 閉じる
		if (m_selectCurrent == static_cast<int>(SelectTitleCurrent::CLOSE))
		{
			isClose = true;
		}
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		// 閉じる
		isClose = true;
	}

	if (isClose)
	{
		m_updateFunc = &SceneOption::FadeOutUpdate;
		m_drawFunc = &SceneOption::DrawFade;
	}
}

void SceneOption::SelectMainUpdate()
{
	if (CursorUtility::CursorUp<SelectMainCurrent>(m_selectCurrent, SelectMainCurrent::MAX)) m_waveCount = 0;
	if (CursorUtility::CursorDown<SelectMainCurrent>(m_selectCurrent, SelectMainCurrent::MAX)) m_waveCount = 0;
	// BGM・SE調整
	if (m_selectCurrent < SOUND_CHECK_CURRENT_MAX)
	{
		SoundChangeUpdate(m_selectCurrent);
	}

	auto& input = Input::GetInstance();
	bool isClose = false;
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		// 閉じる(再開)
		if (m_selectCurrent == static_cast<int>(SelectMainCurrent::RESTART))
		{
			isClose = true;
		}
		// ステージ選択に戻る
		else if (m_selectCurrent == static_cast<int>(SelectMainCurrent::RETURN_STAGE_SELECT))
		{
			m_updateFunc = &SceneOption::CheckMainUpdate;
			m_checkCurrent = 0;
		}
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		// 閉じる(再開)
		isClose = true;
	}

	if (isClose)
	{
		m_updateFunc = &SceneOption::FadeOutUpdate;
		m_drawFunc = &SceneOption::DrawFade;
	}
}

void SceneOption::CheckMainUpdate()
{
	if (CursorUtility::CursorUp<CheckMainCurrent>(m_checkCurrent, CheckMainCurrent::MAX, Command::BTN_LEFT)) m_waveCount = 0;
	if (CursorUtility::CursorDown<CheckMainCurrent>(m_checkCurrent, CheckMainCurrent::MAX, Command::BTN_RIGHT)) m_waveCount = 0;

	auto& input = Input::GetInstance();
	bool isCancel = false;
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		// ステージ選択シーンへ
		if (m_checkCurrent == static_cast<int>(CheckMainCurrent::RETURN))
		{
			auto next = std::make_shared<SceneStageSelect>();
			m_scnMgr.Change(next);
			m_updateFunc = &SceneOption::FadeOutUpdate;
		}
		// キャンセル
		else if (m_checkCurrent == static_cast<int>(CheckMainCurrent::CANCEL))
		{
			isCancel = true;
		}
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		isCancel = true;
	}

	if (isCancel)
	{
		m_updateFunc = &SceneOption::CheckMainUpdate;
	}
}

void SceneOption::SoundChangeUpdate(int current)
{
	auto& input = Input::GetInstance();
	auto& sndMgr = SoundManager::GetInstance();

	// それぞれに合った方の現在のボリュームを取得
	int volume = 0;
	if (current == static_cast<int>(SoundChange::BGM))
	{
		volume = sndMgr.GetBgmVol();
	}
	else if (current == static_cast<int>(SoundChange::SE))
	{
		volume = sndMgr.GetSeVol();
	}

	// 音量減少
	if (input.IsRepeat(Command::BTN_LEFT, REPEATE_INTERVAL, START_REPEATE_INTERVAL))
	{
		volume -= CHANGE_VOLUME;
	}
	// 音量増加
	if (input.IsRepeat(Command::BTN_RIGHT, REPEATE_INTERVAL, START_REPEATE_INTERVAL))
	{
		volume += CHANGE_VOLUME;
	}

	// それぞれに合わせて変更したボリュームを設定
	if (current == static_cast<int>(SoundChange::BGM))
	{
		sndMgr.ChangeBgmVol(volume);
	}
	else if (current == static_cast<int>(SoundChange::SE))
	{
		sndMgr.ChangeSeVol(volume);
	}
}

void SceneOption::DrawFade() const
{
	int y = Easing::Lerp(-DRAW_OUT_VAL, Game::CENTER_Y, m_fadeCount);
	DrawRotaGraphFast(Game::CENTER_X, y, 1.0f, 0.0f, m_files.at(FILE_WINDOW)->GetHandle(), true);
}

void SceneOption::DrawCommon() const
{
	int selectH = m_files.at(FILE_COMMON_SELECT_FRAME)->GetHandle();
	int notSelectH = m_files.at(FILE_COMMON_FRAME)->GetHandle();
	DrawRotaGraphFast(Game::CENTER_X, Game::CENTER_Y, 1.0f, 0.0f, m_files.at(FILE_WINDOW)->GetHandle(), true);
	UIUtility::DrawFrameAndStr(m_uiList, FILE_SIZE_FRAME, 0.0f, FONT_SIZE, m_selectCurrent, m_selectStrList.at(m_openScene), selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT, UIUtility::DrawStrType::WAVE, WAVE_SIZE, m_waveCount * WAVE_SPEED);
	auto& sndMgr = SoundManager::GetInstance();
	DrawSoundBar(DRAW_SOUND_Y, sndMgr.GetBgmVol(), sndMgr.GetBgmVolRate());
	DrawSoundBar(DRAW_SOUND_Y + DRAW_SOUND_Y_INTERVAL, sndMgr.GetSeVol(), sndMgr.GetSeVolRate());
}

void SceneOption::DrawSoundBar(int y, int volume, float volumeRate) const
{
	DrawRotaGraphFast(DRAW_SOUND_X, y, 1.0f, 0.0f, m_files.at(FILE_SOUND_BAR)->GetHandle(), true);
	int x = DRAW_SOUND_POINT_X + static_cast<int>(DRAW_SOUND_POINT_X_INTERVAL * volumeRate);
	DrawRotaGraphFast(x, y, 1.0f, 0.0f, m_files.at(FILE_VOLUME_POINT)->GetHandle(), true);
	y -= DRAW_SOUND_TXT_FRAME_SUB_Y;
	int w, h;
	GetGraphSize(m_files.at(FILE_COMMON_FRAME)->GetHandle(), &w, &h);
	w = static_cast<int>(w * 0.5f * FILE_SIZE_SOUND_TXT_FRAME_W);
	h = static_cast<int>(h * 0.5f * FILE_SIZE_SOUND_TXT_FRAME_H);
	DrawExtendGraph(DRAW_SOUND_TXT_FRAME_X - w, y - h, DRAW_SOUND_TXT_FRAME_X + w, y + h, m_files.at(FILE_COMMON_FRAME)->GetHandle(), true);
	DrawFormatStringToHandle(DRAW_SOUND_TXT_X, y - DRAW_SOUND_TXT_SUB_Y, COLOR_SOUND, FontManager::GetInstance().GetHandle(FONT_SIZE_SOUND), L"%3d", volume);
}

void SceneOption::DrawCheck() const
{
	DrawRotaGraphFast(Game::CENTER_X, Game::CENTER_Y, 1.0f, 0.0f, m_files.at(FILE_WINDOW)->GetHandle(), true);
	DrawString(480, 360, L"キャンセル", 0xffffff);
	DrawString(640, 360, L"ステージ選択へ", 0xffffff);
}
