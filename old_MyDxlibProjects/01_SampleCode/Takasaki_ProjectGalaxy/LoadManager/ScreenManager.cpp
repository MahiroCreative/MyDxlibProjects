#include "ScreenManager.h"

ScreenManager::ScreenManager()
{
}

ScreenManager::~ScreenManager()
{
}

ScreenManager& ScreenManager::GetInstance()
{
	static ScreenManager info;
	return info;
}

int ScreenManager::GetScreenData(const char* screenname,int width, int height)
{
	if (m_pathAndScreenInfoes.find(screenname) == m_pathAndScreenInfoes.end())
	{
		ScreenInfo m = ScreenInfo();
		
		m.handle = DxLib::MakeScreen(width, height,true);
		m.used = false;
		m_pathAndScreenInfoes[screenname] = m;
		return m.handle;
	}
	else {
		m_pathAndScreenInfoes[screenname].used = true;
		return m_pathAndScreenInfoes[screenname].handle;
	}
}
