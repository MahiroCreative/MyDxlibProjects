#pragma once
#include<map>
#include<string>
#include"DxLib.h"
struct SoundInfo {
	int handle;//ハンドル
	bool used;//一度以上使用済みか？
};
class SoundManager
{
private:
	std::map<std::string, SoundInfo> m_pathAndSoundInfoes;
	int m_soundVol;

public:
	SoundManager();
	~SoundManager();

	int GetSoundVol() const { return m_soundVol; }

	static SoundManager& GetInstance();
	void ChangeSoundVolume(int vol);
	int GetSoundData(const char* filepath);

};

