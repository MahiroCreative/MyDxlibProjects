#pragma once
#include <DxLib.h>
#include <cassert>
#include <unordered_map>
#include <list>
#include <string>
#include "File.h"

namespace
{
	// ファイルID
	const wchar_t* const FILE_NUM = L"I_Numbers";
	// 切り取り行数
	constexpr int SRC_ROW_NUM = 4;
	// 切り取り列数
	constexpr int SRC_LINE_NUM = 4;
}

class NumUtility
{
private:
	NumUtility() :
		m_file(nullptr),
		m_handle(-1)
	{
		m_currentList[L"."] = 10;
		m_currentList[L":"] = 11;
	}

	NumUtility(const NumUtility& mgr) = delete;
	void operator= (const NumUtility& mgr) = delete;

public:
	static NumUtility& GetInstance()
	{
		static NumUtility instance;
		return instance;
	}

	void Init()
	{
		m_handle = FileManager::GetInstance().Load(FILE_NUM, true)->GetHandle();
	}

	/// <summary>
	/// 数字の画像描画(1桁のみ)(整数)
	/// </summary>
	/// <param name="x">中心座標</param>
	/// <param name="y">中心座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="num">数字(-1だと下の文字を使う)</param>
	/// <param name="c">使用可能文字(".", ":")</param>
	void DrawNumber(int x, int y, float size, int num, const wchar_t* const c = L"")
	{
		assert(-1 <= num && num < 10 && "数字が範囲外です");
		if (num < 0)
		{
			num = m_currentList.at(c);
		}

		int srcX = SRC_SIZE_X * (num % SRC_ROW_NUM);
		int srcY = SRC_SIZE_Y * (num / SRC_LINE_NUM);
		DrawRectRotaGraphFast(x, y, srcX, srcY, SRC_SIZE_X, SRC_SIZE_Y, size, 0.0f, m_handle, true);
	}
	/// <summary>
	/// 数字の画像描画(1桁以上)(整数)
	/// </summary>
	/// <param name="x">中心座標</param>
	/// <param name="y">中心座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="num">数字</param>
	/// <param name="fillNum">0埋め数</param>
	void DrawNumberMore(int x, int y, float size, unsigned int num, int fillNum = 0)
	{
		std::list<unsigned int> list;
		// 一番桁から数字を取得していく
		while (true)
		{
			list.emplace_front(num % 10);
			// 次の桁に移行
			num /= 10;
			// 数字が0になると終了
			if (num <= 0) break;
		}
		if (static_cast<int>(list.size()) < fillNum) for (int i = static_cast<int>(list.size()); i < fillNum; ++i) list.emplace_front(0);

		int addSize = static_cast<int>(SRC_SIZE_X * size);
		x = x - static_cast<int>(addSize * list.size() * 0.5f);
		for (auto& item : list)
		{
			DrawNumber(x, y, size, item);
			x += addSize;
		}
	}

public:
	// 切り取りサイズ
	static constexpr int SRC_SIZE_X = 66;
	static constexpr int SRC_SIZE_Y = 130;

private:
	std::shared_ptr<FileBase> m_file;
	std::unordered_map<const wchar_t*, int> m_currentList;
	int m_handle;
};