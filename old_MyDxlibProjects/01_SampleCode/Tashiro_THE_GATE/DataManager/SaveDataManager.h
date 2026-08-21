#pragma once
#include <unordered_map>
#include <vector>
#include <string>

struct SaveData
{
	bool isExist = false;
	int totalTime = 0;
	std::unordered_map<std::wstring, bool> isClear;
	std::unordered_map<std::wstring, int> cpNo;
	std::unordered_map<std::wstring, int> playTime;
	std::unordered_map<std::wstring, int> clearTime;
};
using SaveDataTable_t = std::vector<SaveData>;

class SaveDataManager final
{
private:
	SaveDataManager();

	SaveDataManager(const SaveDataManager& mgr) = delete;
	void operator= (const SaveDataManager& mgr) = delete;

public:
	static SaveDataManager& GetInstance();

	/// <summary>
	/// 読み込み
	/// </summary>
	void Load();
	/// <summary>
	/// 保存
	/// </summary>
	void Save() const;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="no">セーブデータ番号</param>
	void Initialized(int no);

	/// <summary>
	/// 使用セーブデータの決定
	/// </summary>
	/// <param name="no">セーブデータ番号</param>
	void DecideUseSaveNo(int no) { m_useSaveNo = no; }

	/// <summary>
	/// 総プレイ時間とステージプレイ時間の更新
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	void UpdateTime(const wchar_t* const stageName);
	/// <summary>
	/// チェックポイントの更新
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <param name="checkNo">更新予定のCP番号</param>
	/// <returns>true:更新した /false: 更新していない</returns>
	bool UpdateCheckPoint(const wchar_t* const stageName, int checkNo);

	/// <summary>
	/// ステージをクリアしたことにする
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <param name="clearTime">クリアタイム</param>
	void OnClear(const wchar_t* const stageName, int clearTime);

	/// <summary>
	/// セーブデータの数を取得する
	/// </summary>
	/// <returns>セーブデータ数</returns>
	int GetSaveDataNum() const;
	/// <summary>
	/// クリアステージ数を取得する
	/// </summary>
	/// <param name="saveNo">セーブデータ番号</param>
	/// <returns>クリアステージ数</returns>
	int GetClearStageNum(int saveNo) const;
	/// <summary>
	/// そのセーブデータのゲームプレイ時間を取得する
	/// </summary>
	/// <param name="saveNo">セーブデータ番号</param>
	/// <returns>プレイ時間</returns>
	int GetPlayTime(int saveNo) const { return m_dataTable.at(saveNo).totalTime; }
	/// <summary>
	/// 現在使用しているセーブデータ番号を取得する
	/// </summary>
	/// <returns>セーブデータ番号</returns>
	int GetUseSaveNo() const { return m_useSaveNo; }
	/// <summary>
	/// ステージでのチェックポイント番号を取得
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <returns>チェックポイント番号</returns>
	int GetCpNo(const wchar_t* const stageName) const { return m_dataTable.at(m_useSaveNo).cpNo.at(stageName); }
	/// <summary>
	/// ステージのプレイ時間を取得
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <returns>プレイ時間</returns>
	int GetStagePlayTime(const wchar_t* const stageName) const { return m_dataTable.at(m_useSaveNo).playTime.at(stageName); }
	/// <summary>
	/// ステージのクリア時間を取得
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <returns>クリア時間</returns>
	int GetStageClearTime(const wchar_t* const stageName) const { return m_dataTable.at(m_useSaveNo).clearTime.at(stageName); }
	/// <summary>
	/// セーブデータが存在するか
	/// </summary>
	/// <param name="no">セーブデータ番号</param>
	/// <returns>true: 存在する/ false: 存在しない</returns>
	bool IsExist(int no) { return m_dataTable.at(no).isExist; }
	/// <summary>
	/// ステージをクリアしているか
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <returns>true: クリアしている/ false: クリアしていない</returns>
	bool IsClear(const wchar_t* const stageName) const { return m_dataTable.at(m_useSaveNo).isClear.at(stageName); }
	/// <summary>
	/// ステージを解放しているか
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	/// <returns>true: 解放している/ false: 解放していない</returns>
	bool IsReleaseStage(const wchar_t* const stageName) const;

private:
	void LoadFile(int handle);

private:
	int m_useSaveNo;
	SaveDataTable_t m_dataTable;
};
