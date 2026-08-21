#include "ModelManager.h"
#include<string>
#include<vector>

using namespace std;
ModelManager::ModelManager()
{
}

ModelManager::~ModelManager()
{
}

ModelManager& ModelManager::GetInstance()
{
	static ModelManager info;
	return info;
}

int ModelManager::GetModelData(const char* modelname)
{

	if (m_pathAndModelInfoes.find(modelname) == m_pathAndModelInfoes.end())
	{
		ModelInfo m = ModelInfo();
		string fileName = "Data/Model/";
		m.handle = DxLib::MV1LoadModel((fileName + (string)modelname).c_str());
		m.used = false;
		m_pathAndModelInfoes[modelname] = m;
		return m.handle;
	}
	else {
		m_pathAndModelInfoes[modelname].used = true;
		return m_pathAndModelInfoes[modelname].handle;
	}
}

void ModelManager::DeleteModelData(const char* modelname)
{
	string fileName = "Data/Model/";
	DxLib::DeleteGraph(m_pathAndModelInfoes[modelname].handle);
	m_pathAndModelInfoes.erase(modelname);
}
