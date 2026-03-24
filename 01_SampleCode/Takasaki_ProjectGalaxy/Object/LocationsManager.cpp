#include "LocationsManager.h"
#include"DxLib.h"

LocationsManager& LocationsManager::GetInstance()
{
	static LocationsManager info;
	return info;
}
std::vector<LocationsManager::LocationData> LocationsManager::LoadLocations(const char* filename)
{
	std::string filePath = "Data/Info/";
	int handle = FileRead_open((filePath + (std::string)filename).c_str());
	int dataCnt = 0;
	FileRead_read(&dataCnt, sizeof(dataCnt), handle);
	
	data[filename].resize(dataCnt);
	for (auto& loc : data[filename])
	{
		//オブジェクト名ロード
		byte nameCnt = 0;
		FileRead_read(&nameCnt, sizeof(nameCnt), handle);
		loc.name.resize(nameCnt);
		FileRead_read(loc.name.data(), sizeof(char) * loc.name.size(), handle);
		//タグ
		FileRead_read(loc.tag.data(), sizeof(char) * loc.tag.size(), handle);
		//座標情報
		FileRead_read(&loc.pos, sizeof(loc.pos), handle);

		//回転情報
		FileRead_read(&loc.rot, sizeof(loc.rot), handle);

		//スケール情報
		FileRead_read(&loc.pos, sizeof(loc.scale), handle);

	}
	FileRead_close(handle);

	return data[filename];
}
