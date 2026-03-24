#include "GraphManager.h"
#include<string>
#include<vector>

using namespace std;

GraphManager::GraphManager()
{
}

GraphManager::~GraphManager()
{
}

GraphManager& GraphManager::GetInstance()
{
	static GraphManager info;
	return info;
}

int GraphManager::GetGraphData(const char* graphname)
{
	if (m_pathAndGraphInfoes.find(graphname) == m_pathAndGraphInfoes.end())
	{
		GraphInfo m = GraphInfo();
		string fileName = "Data/Image/";
		m.handle = DxLib::LoadGraph((fileName + (string)graphname).c_str());
		m.used = false;
		m_pathAndGraphInfoes[graphname] = m;
		return m.handle;
	}
	else {
		m_pathAndGraphInfoes[graphname].used = true;
		return m_pathAndGraphInfoes[graphname].handle;
	}
}

void GraphManager::DeleteGraphData(const char* graphname)
{
	string fileName = "Data/Image/";
	DxLib::DeleteGraph(m_pathAndGraphInfoes[graphname].handle);
	m_pathAndGraphInfoes.erase(graphname);
}
