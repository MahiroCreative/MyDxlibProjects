#include "SceneResult.h"
#include <DxLib.h>
#include <algorithm>
#include "TimeUtility.h"
#include "NumUtility.h"
#include "Game.h"
#include "File.h"
#include "Application.h"
#include "SceneManager.h"
#include "RankingDataManager.h"
#include "StageDataManager.h"

namespace
{
	// ファイルID
	const wchar_t* const FILE_RANK_S = L"I_RankS";
	const wchar_t* const FILE_RANK_A = L"I_RankA";
	const wchar_t* const FILE_RANK_B = L"I_RankB";
	const wchar_t* const FILE_RANK_C = L"I_RankC";
	const wchar_t* const FILE_WINDOW = L"I_ClearWindow";
	const wchar_t* const FILE_TIME_FRAME = L"I_ClearTimeFrame";
	const wchar_t* const FILE_RANKING_UPDATE = L"I_RankingUpdate";

	/* ウィンドウ */
	// 描画位置
	constexpr int DRAW_WINDOW_X = 900;
	constexpr int DRAW_WINDOW_Y = Game::CENTER_Y;
	/* タイムフレーム */
	// 描画位置
	constexpr int DRAW_TIME_FRAME_X = DRAW_WINDOW_X;
	constexpr int DRAW_TIME_FRAME_Y = 400;
	// タイム描画位置
	constexpr int DRAW_TIME_X = 945;
	constexpr int DRAW_TIME_Y = 410;
	// フォントサイズ
	constexpr int FONT_SIZE = 64;
	// 文字カラー
	constexpr unsigned int COLOR_TIME = 0xffffff;
	/* ランキング更新 */
	constexpr int DRAW_RANKING_X = DRAW_WINDOW_X;
	constexpr int DRAW_RANKING_Y = 550;


	// 決定までのカウント
	constexpr int DECIDE_COUNT = 20;
	// 移動するカウント
	constexpr int MOVE_COUNT = 20;
	

}

SceneResult::SceneResult(const wchar_t* const stageName, const int clearTime) :
	SceneBase(SceneKind::CLEAR),
	m_stageName(stageName),
	m_clearTime(clearTime),
	m_processFunc(&SceneResult::TimeUpdate),
	m_moveCount{ 0 },
	m_count(0),
	m_indexDecide(0),
	m_scale(0),
	m_isDecide{ false },
	m_isMove{ false },
	m_isAllDecide(false),
	m_isAllMoved(false),
	m_isRankingUpdate(false)
{
}

void SceneResult::AsyncInit()
{
	// ファイル読み込み
	auto& fileMgr = FileManager::GetInstance();
	m_files[FILE_RANK_S] = fileMgr.Load(FILE_RANK_S);
	m_files[FILE_RANK_A] = fileMgr.Load(FILE_RANK_A);
	m_files[FILE_RANK_B] = fileMgr.Load(FILE_RANK_B);
	m_files[FILE_RANK_C] = fileMgr.Load(FILE_RANK_C);
	m_files[FILE_WINDOW] = fileMgr.Load(FILE_WINDOW);
	m_files[FILE_TIME_FRAME] = fileMgr.Load(FILE_TIME_FRAME);
	m_files[FILE_RANKING_UPDATE] = fileMgr.Load(FILE_RANKING_UPDATE);
}

void SceneResult::Init()
{
}

void SceneResult::End()
{
}

void SceneResult::Update(bool isFade)
{
	(this->*m_processFunc)();
	if (isFade) return;
	SelectUpdate();
}

void SceneResult::Draw() const
{
	// ウィンドウ
	DrawRotaGraphFast(DRAW_WINDOW_X, DRAW_WINDOW_Y, 1.0f, 0.0f, m_files.at(FILE_WINDOW)->GetHandle(), true);
	// タイムフレーム
	DrawRotaGraphFast(DRAW_TIME_FRAME_X, DRAW_TIME_FRAME_Y, 1.0f, 0.0f, m_files.at(FILE_TIME_FRAME)->GetHandle(), true);

	DrawTime();
	DrawRank();
	DrawRankingUpdate();
}

void SceneResult::TimeUpdate()
{
	if (m_isAllMoved) return;

	bool isAllMoved = true;
	for (int i = 0; i < static_cast<int>(DecideTime::MAX); ++i)
	{
		// 動かないなら次へ
		if (!m_isMove->at(i))
		{
			isAllMoved = false;
			continue;
		}

		auto& count = m_moveCount->at(i);
		++count;
		if (count > MOVE_COUNT)
		{
			count = MOVE_COUNT;
		}
		else
		{
			isAllMoved = false;
		}
	}
	m_isAllMoved = isAllMoved;

	if (m_isAllDecide) return;

	++m_count;
	if (m_count > DECIDE_COUNT)
	{
		m_isDecide->at(m_indexDecide) = true;
		m_isMove->at(m_indexDecide) = true;
		m_count = 0;
		++m_indexDecide;
		if (m_indexDecide >= static_cast<int>(DecideTime::MAX))
		{
			m_isAllDecide = true;
		}
	}
}

void SceneResult::StampUpdate()
{
}

void SceneResult::ScalingUpdate()
{
	// ランキングを更新していなければ終了
	if (!m_isRankingUpdate) return;

	constexpr float ADD_SCALE = 0.02f;
	m_scale = std::min<float>(m_scale + ADD_SCALE, 1.0f);
}

void SceneResult::SelectUpdate()
{
}

void SceneResult::DrawTime() const
{
//	NumUtility::GetInstance().DrawNumberMore(DRAW_TIME_X, DRAW_TIME_Y, 1.0f, 1234);
	TimeUtility::DrawTime(DRAW_TIME_X, DRAW_TIME_Y, m_clearTime, 0.875f);

	return;

	//// 全て決定している場合
	//if (m_isAllMoved)
	//{
	//	TimeUtility::DrawTimeCenter(DRAW_TIME_X, DRAW_TIME_Y + DRAW_TIME_STR_SUB_Y, COLOR_TIME, FONT_SIZE, m_clearTime);
	//}
	//// 決定していない場合
	//else
	//{
	//	int y = DRAW_TIME_Y + DRAW_TIME_STR_SUB_Y - static_cast<int>(FONT_SIZE * 0.5f);
	//	auto time = TimeUtility::FrameToTime(m_clearTime);
	//	auto fontH = FontManager::GetInstance().GetHandle(FONT_SIZE);
	//	DrawTime(DRAW_TIME_MIN_X, y, DecideTime::MIN, time.min);
	//	DrawStringToHandle(DRAW_TIME_COLON_X, y, L":", COLOR_TIME, fontH);
	//	DrawTime(DRAW_TIME_SEC_X, y, DecideTime::SEC, time.sec);
	//	DrawStringToHandle(DRAW_TIME_DOT_X, y, L".", COLOR_TIME, fontH);
	//	DrawTime(DRAW_TIME_DEC_X, y, DecideTime::DEC, time.dec);
	//}
}

void SceneResult::DrawTime(int x, int y, DecideTime kind, int time) const
{
	auto& fontMgr = FontManager::GetInstance();
	int size = FONT_SIZE;
	if (kind == DecideTime::DEC)
	{
		size = static_cast<int>(size * FONT_EX_RATE);
		y += static_cast<int>((FONT_SIZE - size) * SLIDE_Y);
	}
	int fontH = fontMgr.GetHandle(size);
	// 決定している場合
	if (m_isDecide->at(static_cast<int>(kind)))
	{
		int count = m_moveCount->at(static_cast<int>(kind));

		if (count < MOVE_COUNT)
		{
			constexpr int FONT_SZIE_TIME_SMALL = 48;
			constexpr int FONT_SZIE_TIME_LARGE = 80;
			int size2 = size;
			if (count < 5)
			{
				size2 = FONT_SZIE_TIME_SMALL;
				if (kind == DecideTime::DEC) size2 = static_cast<int>(size2 * FONT_EX_RATE);
			}
			else if (count < 8)
			{
			}
			else
			{
				size2 = FONT_SZIE_TIME_LARGE;
				if (kind == DecideTime::DEC) size2 = static_cast<int>(size2 * FONT_EX_RATE);
			}
			y -= static_cast<int>((size2 - size) * SLIDE_Y);
			x -= static_cast<int>((size2 - size) * SLIDE_X);
			fontH = fontMgr.GetHandle(size2);
			DrawFormatStringToHandle(x, y, COLOR_TIME, fontH, L"%2d", time);
		}
		else
		{
			DrawFormatStringToHandle(x, y, COLOR_TIME, fontH, L"%2d", time);
		}
	}
	else
	{
		int rand = Application::GetInstance().GetRand(0, 99);
		DrawFormatStringToHandle(x, y, COLOR_TIME, fontH, L"%2d", rand);
	}
}

void SceneResult::DrawRank() const
{
}

void SceneResult::DrawRankingUpdate() const
{
	// ランキングを更新していなければ終了
	if (!m_isRankingUpdate) return;

	float angle = m_scale * 4 * 3.14f;
	DrawRotaGraphFast(DRAW_RANKING_X, DRAW_RANKING_Y, m_scale, angle, m_files.at(FILE_RANKING_UPDATE)->GetHandle(), true);
}
