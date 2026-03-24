#include "SceneStageSelect.h"
#include <DxLib.h>
#include <sstream>
#include "CursorUtility.h"
#include "UIUtility.h"
#include "SaveDataManager.h"
#include "StageDataManager.h"
#include "Game.h"
#include "File.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneMain.h"

namespace
{
	enum class CheckCurrent : int
	{
		CANCEL,
		RETURN,
		MAX
	};

	// ファイルID
	const wchar_t* const FILE_WINDOW_BACK		= L"I_SelectWindowBack";
	const wchar_t* const FILE_WINDOW			= L"I_SelectWindow";
	const wchar_t* const FILE_RELEASE_FRAME		= L"I_ReleaseFrame";
	const wchar_t* const FILE_NOT_RELEASE_FRAME = L"I_NotReleaseFrame";
	const wchar_t* const FILE_SELECT_FRAME		= L"I_StageSelectFrame";
	const wchar_t* const FILE_BEST_TIME_FRAME	= L"I_BestTimeFrame";
	const wchar_t* const FILE_DECORATIVE_FRAME	= L"I_DecorativeFrame";
	const wchar_t* const FILE_DIVIDING_LINE		= L"I_DividingLine";
	const wchar_t* const FILE_SELECT_OBI		= L"I_SelectObi";
	const wchar_t* const FILE_STAGE_IMAGE_BASE	= L"I_StageImage";
	const wchar_t* const FILE_COMMON_FRAME		= L"I_CommonFrame";
	const wchar_t* const FILE_PAD_B				= L"I_PadB";

	/* ウィンドウ */
	// 描画位置
	constexpr int DRAW_WINDOW_Y = 420;
	// 帯描画位置
	constexpr int DRAW_OBI_Y = 64;
	// 分割線描画位置
	constexpr int DRAW_DIVIDING_LINE_X = 927;
	constexpr int DRAW_DIVIDING_LINE_Y = DRAW_WINDOW_Y;
	/* 画像飾り */
	// 描画位置
	constexpr int DRAW_STAGE_IMAGE_X = 927;
	constexpr int DRAW_STAGE_IMAGE_Y = 281;
	// 飾り位置
	constexpr int DRAW_DECORATIVE_FRAME_X_1 = 1166;
	constexpr int DRAW_DECORATIVE_FRAME_Y_1 = 188;
	constexpr int DRAW_DECORATIVE_FRAME_X_2 = 688;
	constexpr int DRAW_DECORATIVE_FRAME_Y_2 = 375;
	/* ステージフレーム */
	// 描画位置
	constexpr int DRAW_STAGE_X = 160;
	constexpr int DRAW_STAGE_Y = 240;
	// 描画間隔
	constexpr int DRAW_STAGE_INTERVAL = 135;
	/*「タイトルへ」 */
	// ベースY描画位置
	constexpr int DRAW_RETURN_TITLE_Y = 688;
	// フレーム描画位置
	constexpr int DRAW_RETURN_TITLE_FRAME_X = 113;
	// フレーム画像サイズ
	constexpr float FILE_SIZE_RETURN_TITLE_FRAME = 0.75f;
	// ボタン描画位置
	constexpr int DRAW_RETURN_TITLE_BUTTON_X = 32;
	// ボタン画像サイズ
	constexpr float FILE_SIZE_RETURN_TITLE_BUTTON = 0.5f;
	// フォントサイズ
	constexpr int FONT_SIZE_RETURN_TITLE = 36;
	// 文字列描画位置
	constexpr int DRAW_REUTRN_TITLE_STR_X = 64;
	constexpr int DRAW_REUTRN_TITLE_STR_Y = DRAW_RETURN_TITLE_Y - static_cast<int>(FONT_SIZE_RETURN_TITLE * 0.5f);
	// 文字カラー
	constexpr unsigned int COLOR_RETURN_TITLE = 0xffffff;

	/* その他 */
	// ステージ列数
	constexpr int STAGE_LINE_NUM = 4;
	// 選択フレームの追加拡大率
	constexpr float ADD_SCALE_STAGE_SELECT = 0.1f;
	// 選択フレームの拡縮サイクル
	constexpr float CYCLE_SCALING_STAGE_SELECT = 3.0f;
	// 文字ウェーブスピード
	constexpr float WAVE_SPEED = 3.0f;
}

SceneStageSelect::SceneStageSelect() :
	SceneBase(SceneKind::STAGE_SELECT),
	m_selectCurrent(0),
	m_count(0)
{
}

void SceneStageSelect::AsyncInit()
{
	// ファイル読み込み
	auto& fileMgr = FileManager::GetInstance();
	m_files[FILE_WINDOW_BACK] = fileMgr.Load(FILE_WINDOW_BACK);
	m_files[FILE_WINDOW] = fileMgr.Load(FILE_WINDOW);
	m_files[FILE_RELEASE_FRAME] = fileMgr.Load(FILE_RELEASE_FRAME);
	m_files[FILE_NOT_RELEASE_FRAME] = fileMgr.Load(FILE_NOT_RELEASE_FRAME);
	m_files[FILE_SELECT_FRAME] = fileMgr.Load(FILE_SELECT_FRAME);
	m_files[FILE_BEST_TIME_FRAME] = fileMgr.Load(FILE_BEST_TIME_FRAME);
	m_files[FILE_DECORATIVE_FRAME] = fileMgr.Load(FILE_DECORATIVE_FRAME);
	m_files[FILE_DIVIDING_LINE] = fileMgr.Load(FILE_DIVIDING_LINE);
	m_files[FILE_SELECT_OBI] = fileMgr.Load(FILE_SELECT_OBI);
	m_files[FILE_COMMON_FRAME] = fileMgr.Load(FILE_COMMON_FRAME);
	m_files[FILE_PAD_B] = fileMgr.Load(FILE_PAD_B, true);
	auto stageNum = StageDataManager::GetInstance().GetStageNum();
	for (int i = 0; i < stageNum; ++i)
	{
		std::wostringstream oss;
		oss << FILE_STAGE_IMAGE_BASE << i;
		const auto& item = oss.str();
		m_files[item] = fileMgr.Load(item.c_str());
	}
}

void SceneStageSelect::Init()
{
}

void SceneStageSelect::Update(bool isFade)
{
	++m_count;
	if (isFade) return;
	SelectUpdate();
}

void SceneStageSelect::End()
{
}

void SceneStageSelect::Draw() const
{
	auto& saveDataMgr = SaveDataManager::GetInstance();
	auto& stageDataMgr = StageDataManager::GetInstance();
	auto& fontMgr = FontManager::GetInstance();
	auto stageNum = stageDataMgr.GetStageNum();
#ifdef _DEBUG
	printf("セーブデータ番号:%d\n", saveDataMgr.GetUseSaveNo() + 1);
#endif
	
	// 背景
	DrawGraph(0, 0, m_files.at(FILE_WINDOW_BACK)->GetHandle(), true);
	// ウィンドウ
	DrawRotaGraphFast(Game::CENTER_X, DRAW_WINDOW_Y, 1.0f, 0.0f, m_files.at(FILE_WINDOW)->GetHandle(), true);
	// 帯
	DrawRotaGraphFast(Game::CENTER_X, DRAW_OBI_Y, 1.0f, 0.0f, m_files.at(FILE_SELECT_OBI)->GetHandle(), true);
	// 分割線
	DrawRotaGraphFast(DRAW_DIVIDING_LINE_X, DRAW_DIVIDING_LINE_Y, 1.0f, 0.0f, m_files.at(FILE_DIVIDING_LINE)->GetHandle(), true);
	// ステージ画像
	std::wostringstream oss;
	oss << FILE_STAGE_IMAGE_BASE << m_selectCurrent;
	DrawRotaGraphFast(DRAW_STAGE_IMAGE_X, DRAW_STAGE_IMAGE_Y, 1.0f, 0.0f, m_files.at(oss.str())->GetHandle(), true);
	// ステージ画像飾り
	DrawRotaGraphFast(DRAW_DECORATIVE_FRAME_X_1, DRAW_DECORATIVE_FRAME_Y_1, 1.0f, 0.0f, m_files.at(FILE_DECORATIVE_FRAME)->GetHandle(), true);
	DrawRotaGraphFast(DRAW_DECORATIVE_FRAME_X_2, DRAW_DECORATIVE_FRAME_Y_2, 1.0f, 180.0f * Game::DEG_2_RAD, m_files.at(FILE_DECORATIVE_FRAME)->GetHandle(), true);
	// ステージ名描画
	UIUtility::DrawWaveStr(Game::CENTER_X, DRAW_OBI_Y, 0xffffff, stageDataMgr.GetStageName(m_selectCurrent), 36, m_count * WAVE_SPEED, 16);

	// ステージフレーム描画
	std::wstring path;
	float scale;
	for (int i = 0; i < stageNum; ++i)
	{
		int x = DRAW_STAGE_X + DRAW_STAGE_INTERVAL * (i % STAGE_LINE_NUM);
		int y = DRAW_STAGE_Y + DRAW_STAGE_INTERVAL * (i / STAGE_LINE_NUM);

		const auto& stageName = stageDataMgr.GetStageName(i);
		bool isSelect = i == m_selectCurrent;
		// ステージ解放されている場合
		if (saveDataMgr.IsReleaseStage(stageName))
		{
			// 現在選んでいる場合
			if (isSelect)
			{
				path = FILE_SELECT_FRAME;
			}
			// 選んでいない場合
			else
			{
				path = FILE_RELEASE_FRAME;
			}
		}
		// ステージ解放されていない場合
		else
		{
			path = FILE_NOT_RELEASE_FRAME;
		}
		// 選択中の場合
		if (isSelect)
		{
			
			scale = 1.0f + std::sin(m_count * Game::DEG_2_RAD * CYCLE_SCALING_STAGE_SELECT) * ADD_SCALE_STAGE_SELECT;
		}
		// 未選択の場合
		else
		{
			scale = 1.0f;
		}
		DrawRotaGraphFast(x, y, scale, 0.0f, m_files.at(path)->GetHandle(), true);
	}

	// 「タイトルへ」描画
	DrawRotaGraphFast(DRAW_RETURN_TITLE_FRAME_X, DRAW_RETURN_TITLE_Y, FILE_SIZE_RETURN_TITLE_FRAME, 0.0f, m_files.at(FILE_COMMON_FRAME)->GetHandle(), true);
	DrawRotaGraphFast(DRAW_RETURN_TITLE_BUTTON_X, DRAW_RETURN_TITLE_Y, FILE_SIZE_RETURN_TITLE_BUTTON, 0.0f, m_files.at(FILE_PAD_B)->GetHandle(), true);
	DrawStringToHandle(DRAW_REUTRN_TITLE_STR_X, DRAW_REUTRN_TITLE_STR_Y, L"タイトルへ", COLOR_RETURN_TITLE, fontMgr.GetHandle(FONT_SIZE_RETURN_TITLE));
}

void SceneStageSelect::SelectUpdate()
{
	auto& input = Input::GetInstance();
	auto& stageDataMgr = StageDataManager::GetInstance();
	auto stageNum = stageDataMgr.GetStageNum();
	// 行数
	const int STAGE_ROW_NUM = stageNum / STAGE_LINE_NUM;
	// 最後の行の列数
	const int LAST_STAGE_LINE_NUM = stageNum % STAGE_LINE_NUM;
	// 最後の行の列数が自分が定義した列数と同じか
	const bool IS_LAST_SAME_SELF_NUM = LAST_STAGE_LINE_NUM == 0;

	// 上下移動
	if (input.IsTriggerd(Command::BTN_UP))
	{
		m_selectCurrent -= STAGE_LINE_NUM;
		// 0未満になった場合
		if (m_selectCurrent < 0)
		{
			// 最後の行が列数と同じなら
			if (IS_LAST_SAME_SELF_NUM)
			{
				// ステージ数分足す
				m_selectCurrent += stageNum;
			}
			// 同じでないなら
			else
			{
				int row = STAGE_ROW_NUM;
				// 列上で何番目かを取得
				auto temp2 = STAGE_LINE_NUM + m_selectCurrent;
				// 番数が最後の行の列数以上なら行を一つ上にする
				if (temp2 >= LAST_STAGE_LINE_NUM) --row;
				// 計算
				m_selectCurrent = row * STAGE_LINE_NUM + temp2;
			}
		}
	}
	if (input.IsTriggerd(Command::BTN_DOWN))
	{
		m_selectCurrent += STAGE_LINE_NUM;
		// ステージ数を超えた場合
		if (m_selectCurrent >= stageNum)
		{
			// カーソルを一番上に戻す
			auto temp = m_selectCurrent - STAGE_LINE_NUM;
			m_selectCurrent = temp % STAGE_LINE_NUM;
		}
	}
	// 左右移動
	CursorUtility::CursorUp(m_selectCurrent, stageNum, Command::BTN_LEFT);
	CursorUtility::CursorDown(m_selectCurrent, stageNum, Command::BTN_RIGHT);
#if false
	// 左右移動(行を変えない)
	if (input.IsTriggerd(Command::BTN_RIGHT))
	{
		// 現在の行での位置を取得
		auto temp = m_selectCurrent % STAGE_LINE_NUM;
		// 最大数を列数にする
		int max = STAGE_LINE_NUM;
		// 最後の行の場合
		if (stageNum - m_selectCurrent < STAGE_LINE_NUM)
		{
			// 最大値を最後の行の列数にする
			max = (stageNum - (m_selectCurrent / STAGE_LINE_NUM) * STAGE_LINE_NUM);
		}
		// 現在の位置を引き
		m_selectCurrent -= temp;
		// その行の列数で計算して
		temp = (temp + 1) % max;
		// 求めた分だけ足す
		m_selectCurrent += temp;
	}
	if (input.IsTriggerd(Command::BTN_LEFT))
	{
		// 現在の行での位置を取得
		auto temp = m_selectCurrent % STAGE_LINE_NUM;
		// 最大数を列数にする
		int max = STAGE_LINE_NUM;
		// 最後の行の場合
		if (stageNum - m_selectCurrent < STAGE_LINE_NUM)
		{
			// 最大値を最後の行の列数にする
			max = (stageNum - (m_selectCurrent / STAGE_LINE_NUM) * STAGE_LINE_NUM);
		}
		// 現在の位置を引き
		m_selectCurrent -= temp;
		// その行の列数で計算して
		temp = (max + temp - 1) % max;
		// 求めた分だけ足す
		m_selectCurrent += temp;
	}
#endif

	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		// スタート
		const auto& stageName = stageDataMgr.GetStageName(m_selectCurrent);
		auto next = std::make_shared<SceneMain>(stageName);
		m_scnMgr.Change(next);
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		// タイトルへ
		auto next = std::make_shared<SceneTitle>();
		m_scnMgr.Change(next);
	}
}
