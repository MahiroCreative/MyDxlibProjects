#include "StageDataManager.h"
#include <cassert>
#include "StringUtility.h"
#include "FileManager.h"

namespace
{
	const wchar_t* const FILE_PAHT = L"Data/Master/StageDataMaster.csv";
	enum StageDataCsv : int
	{
		NAME,
		TIME_S,
		TIME_A,
		TIME_B,
		ONE_HAND,
		GATE_POS,
		GATE_NORM,
		GATE_DIR,
		GATE_TAG
	};
	const std::unordered_map<std::wstring, ObjectTag> GATE_TAG_TABLE =
	{
		{ L"SystemWall", ObjectTag::WALL },
		{ L"Floor", ObjectTag::FLOOR },
	};
}

StageDataManager::StageDataManager()
{
}

StageDataManager& StageDataManager::GetInstance()
{
	static StageDataManager instance;
	return instance;
}

void StageDataManager::Load()
{
	auto& mgr = FileManager::GetInstance();

	auto table = mgr.LoadCsv(FILE_PAHT);
	for (auto& info : table)
	{
		auto& name = info[StageDataCsv::NAME];
		m_nameTable.push_back(name);

		auto& data = m_dataTable[name];
		data.rankTime[RankKind::S] = StringUtility::CsvToTime(info[StageDataCsv::TIME_S]);
		data.rankTime[RankKind::A] = StringUtility::CsvToTime(info[StageDataCsv::TIME_A]);
		data.rankTime[RankKind::B] = StringUtility::CsvToTime(info[StageDataCsv::TIME_B]);
		data.isOneHnad = static_cast<bool>(std::stoi(info[StageDataCsv::ONE_HAND]));
		if (data.isOneHnad)
		{
			data.gatePos = StringUtility::CsvToVec3(info[StageDataCsv::GATE_POS]);
			data.gateNorm = StringUtility::CsvToVec3(info[StageDataCsv::GATE_NORM]);
			data.gateDir = StringUtility::CsvToVec3(info[StageDataCsv::GATE_DIR]);
			data.gateTag = GATE_TAG_TABLE.at(info[StageDataCsv::GATE_TAG]);
		}
	}
}

int StageDataManager::GetStageNo(const wchar_t* const stageName) const
{
	for (int i = 0; i < m_nameTable.size(); ++i)
	{
		const auto& name = m_nameTable.at(i);
		if (name == stageName) return i;
	}
	assert(false && "存在しないステージ名です");
	return -1;
}

RankKind StageDataManager::GetRank(const wchar_t* const stageName, int time) const
{
	// TODO: return ステートメントをここに挿入します
	const std::vector<RankKind> ORDER =
	{
		RankKind::S,
		RankKind::A,
		RankKind::B,
	};

	for (auto& rank : ORDER)
	{
		const auto& info = m_dataTable.at(stageName).rankTime.at(rank);
		// タイムがランクタイム未満なら達成していることとし、現在のランクを返す
		if (time <= info) return rank;
	}

	return RankKind::C;
}
