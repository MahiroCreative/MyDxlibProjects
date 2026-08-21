#pragma once
#include<string>
#include<vector>
#include<unordered_map>
#include"Vec3.h"

class LocationsManager
{
public:
	struct LocationData
	{
		std::string name;
		std::string tag;
		Vec3 pos;
		Vec3 rot;
		Vec3 scale;
	};

	std::unordered_map <std::string, std::vector<LocationData>> data;

	static LocationsManager& GetInstance();

	std::vector<LocationData> LoadLocations(const char* filename);
	std::vector<LocationData> m_locationData;
};

