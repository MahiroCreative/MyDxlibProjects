#pragma once
#include <string>

class FileManager;

/// <summary>
/// ファイルベース
/// </summary>
class FileBase
{
friend FileManager;

public:
	FileBase(FileManager& mgr);
	virtual ~FileBase();

	/// <summary>
	/// ハンドルの取得
	/// </summary>
	/// <returns>ハンドル</returns>
	int GetHandle() const { return m_handle; }
	/// <summary>
	/// ファイル名の取得(パスでない為注意)
	/// </summary>
	/// <returns>ファイル名</returns>
	const std::wstring& GetName() const { return m_name; }

private:
	/// <summary>
	/// ハンドルの削除
	/// </summary>
	virtual void Delete() abstract;
	void End() { m_isEnd = true; }

protected:
	FileManager& m_mgr;
	std::wstring m_name;
	int m_handle;
	int m_refCount;
	bool m_isEternal;
	bool m_isEnd;
};
