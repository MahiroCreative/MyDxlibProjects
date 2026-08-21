#pragma once
#include<map>
#include<string>
#include<EffekseerForDXLib.h>
struct EffectInfo {
	int handle;//ハンドル
	bool used;//一度以上使用済みか？
};
class EffectManager
{
private:
	std::map<std::string, EffectInfo> m_pathAndEffectInfoes;
	
public:
	EffectManager();
	~EffectManager();

	static EffectManager& GetInstance();
	int GetEffectData(const char* filepath);
	void DeleteEffectData(const char* effectname);

	
};

