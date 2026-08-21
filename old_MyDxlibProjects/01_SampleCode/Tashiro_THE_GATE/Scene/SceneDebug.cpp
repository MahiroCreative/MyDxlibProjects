#ifdef _DEBUG
#include "SceneDebug.h"
#include <DxLib.h>
#include <string>
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneStageSelect.h"
#include "SceneMain.h"
#include "SceneResult.h"
#include "CursorUtility.h"
#include "StageDataManager.h"

namespace
{
	std::wstring NAME[] =
	{
		L"Title",
		L"StageSelect",
		L"Main",
		L"Result",
		L"Option",
		L"Debug",
	};
}

SceneDebug::SceneDebug() :
	SceneBase(SceneKind::DEBUG),
	m_next(0)
{
}

void SceneDebug::AsyncInit()
{
}

void SceneDebug::Init()
{
}

void SceneDebug::End()
{
}

void SceneDebug::Update(bool isFade)
{
	if (isFade) return;
	CursorUtility::CursorUp<SceneKind>(m_next, SceneKind::MAX);
	CursorUtility::CursorDown<SceneKind>(m_next, SceneKind::MAX);

	auto& input = Input::GetInstance();
	if (input.IsTriggerd(KEY_INPUT_RETURN) || input.IsTriggerd(Command::BTN_OK))
	{
		std::shared_ptr<SceneBase> next;

		if (static_cast<SceneKind>(m_next) == SceneKind::TITLE)
		{
			next = std::make_shared<SceneTitle>();
		}
		else if (static_cast<SceneKind>(m_next) == SceneKind::STAGE_SELECT)
		{
			next = std::make_shared<SceneStageSelect>();
		}
		else if (static_cast<SceneKind>(m_next) == SceneKind::MAIN)
		{
			auto& stageDataMgr = StageDataManager::GetInstance();
			auto stageName = stageDataMgr.GetStageName(1);
			next = std::make_shared<SceneMain>(stageName);
		}
		else if (static_cast<SceneKind>(m_next) == SceneKind::CLEAR)
		{
			auto& stageDataMgr = StageDataManager::GetInstance();
			auto stageName = stageDataMgr.GetStageName(1);
			next = std::make_shared<SceneResult>(stageName, 3600 * 12 + 60 * 34 + 56);
			
		}

		m_scnMgr.Change(next);
	}
}

void SceneDebug::Draw() const
{
	DrawString(8, 8, L"SceneDebug", 0xffffff);

	int y = 32;
	for (int i = 0; i < static_cast<int>(SceneKind::MAX); ++i)
	{
		if (i == static_cast<int>(m_next))
		{
			DrawBox(16, y - 4, 96, y + 20, 0xff0000, true);
		}

		DrawFormatString(16, y, 0xffffff, NAME[i].c_str());
		if (i >= static_cast<int>(SceneKind::OPTION))
		{
			DrawFormatString(128, y, 0xffffff, L"使えません");
		}
		

		y += 24;
	}
}
#endif