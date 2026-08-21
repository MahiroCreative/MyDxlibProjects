#include <DxLib.h>
#include <cassert>
#include "FontManager.h"

namespace
{
	const wchar_t* const kFontPath = L"Font/Kaisotai-Next-UP-B.otf";
	const wchar_t* const kFontName = L"廻想体 ネクスト UP B";
}

FontManager::FontManager() :
	m_fontPath()
{}

FontManager::~FontManager()
{
	// 作ったフォントデータを全て削除
	for (auto& handle : m_handle)
	{
		DeleteFontToHandle(handle.second);
	}

	if (RemoveFontResourceEx(m_fontPath, FR_PRIVATE, NULL)) {
	}
	else {
		MessageBox(NULL, L"remove failure", L"", MB_OK);
	}
}

FontManager& FontManager::GetInstance()
{
	static FontManager instance;
	return instance;
}

void FontManager::Init()
{
	// 読み込むフォントファイルのパス
	m_fontPath = kFontPath;
	if (AddFontResourceEx(m_fontPath, FR_PRIVATE, NULL) > 0) 
	{
	}
	else 
	{
		// フォント読込エラー処理
		MessageBox(NULL, L"フォント読込失敗", L"", MB_OK);
	}

	// 使いそうなサイズを作っておく
	m_handle[16] = CreateFontToHandle(kFontName, 16, -1);
	m_handle[24] = CreateFontToHandle(kFontName, 24, -1);
	m_handle[28] = CreateFontToHandle(kFontName, 28, -1);
	m_handle[32] = CreateFontToHandle(kFontName, 32, -1);
	m_handle[36] = CreateFontToHandle(kFontName, 36, -1);
	m_handle[42] = CreateFontToHandle(kFontName, 42, -1);
	m_handle[48] = CreateFontToHandle(kFontName, 48, -1);
	m_handle[56] = CreateFontToHandle(kFontName, 56, -1);
	m_handle[60] = CreateFontToHandle(kFontName, 60, -1);
	m_handle[64] = CreateFontToHandle(kFontName, 64, -1);
	m_handle[80] = CreateFontToHandle(kFontName, 80, -1);
}

int FontManager::GetHandle(int size) const
{
	auto it = m_handle.find(size);
	if (it == m_handle.end())
	{
		assert(false && L"そのフォントサイズは存在しません");
		return m_handle.at(32);
	}

	return it->second;
}
