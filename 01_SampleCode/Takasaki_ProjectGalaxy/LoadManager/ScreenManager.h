#pragma once
#include<map>
#include<string>
#include"DxLib.h"

struct ScreenInfo
{
	int handle;
	bool used;
};
class ScreenManager
{
private:
	std::map<std::string, ScreenInfo> m_pathAndScreenInfoes;

public:
	ScreenManager();
	~ScreenManager();

	static ScreenManager& GetInstance();
	int GetScreenData(const char* screenname, int width, int height);
};

