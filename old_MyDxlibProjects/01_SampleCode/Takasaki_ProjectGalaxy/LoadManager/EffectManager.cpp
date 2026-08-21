#include "EffectManager.h"
#include<string>
#include<vector>

using namespace std;

EffectManager::EffectManager()
{

}

EffectManager::~EffectManager()
{
}

EffectManager& EffectManager::GetInstance()
{
	static EffectManager info;
	return info;
}

int EffectManager::GetEffectData(const char* effectname)
{
	if (m_pathAndEffectInfoes.find(effectname) == m_pathAndEffectInfoes.end())
	{
		EffectInfo m = EffectInfo();
		string fileName = "Effect/";
		SetUseASyncLoadFlag(false);
		m.handle = LoadEffekseerEffect((fileName + (string)effectname).c_str());
		SetUseASyncLoadFlag(true);
		m.used = false;
		m_pathAndEffectInfoes[effectname] = m;
		return m.handle;
	}
	else {
		m_pathAndEffectInfoes[effectname].used = true;
		return m_pathAndEffectInfoes[effectname].handle;
	}
}

void EffectManager::DeleteEffectData(const char* effectname)
{
	string fileName = "Effect/";
	DeleteEffekseerEffect(m_pathAndEffectInfoes[effectname].handle);
	m_pathAndEffectInfoes.erase(effectname);
}
