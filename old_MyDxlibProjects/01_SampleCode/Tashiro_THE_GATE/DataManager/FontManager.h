#pragma once
#include <unordered_map>
#include <windows.h>

/// <summary>
/// フォントを扱うクラス
/// </summary>
class FontManager
{
private:
	FontManager();

	FontManager(const FontManager& font) = delete;
	void operator= (const FontManager& font) = delete;

public:
	~FontManager();

	static FontManager& GetInstance();

	/// <summary>
	/// フォントデータの作成
	/// </summary>
	void Init();

	/// <summary>
	/// サイズに対応したハンドルを取得
	/// </summary>
	/// <param name="size">フォントのサイズ</param>
	/// <returns>ハンドル</returns>
	int GetHandle(int size) const;

private:
	LPCWSTR m_fontPath;

	// フォントのハンドル
	// Key:フォントのサイズ　Value:フォントハンドル
	std::unordered_map<int, int> m_handle;
};

