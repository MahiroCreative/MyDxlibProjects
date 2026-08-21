#pragma once
#include <sstream>
#include <iomanip>
#include "FontManager.h"
#include "UIUtility.h"
#include "NumUtility.h"

namespace
{
	constexpr float SLIDE_X = 0.31f;
	constexpr float SLIDE_Y = 0.8f;
	constexpr float FONT_EX_RATE = 0.75f;
}

class TimeUtility
{
public:
	int min;
	int sec;
	int dec;

public:
	static TimeUtility FrameToTime(int frame)
	{
		TimeUtility time;
		// 分に変換
		time.min = frame / 3600;
		frame %= 3600;
		// 秒に変換
		time.sec = frame / 60;
		frame %= 60;
		// 少数以下に変換
		time.dec = static_cast<int>(frame / 60.0f * 100);
		return time;
	}
	/// <summary>
	/// 時間(mm:ss.dd)の描画(左上基準)
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="frame">時間</param>
	/// <param name="size">画像サイズ(整数部分)</param>
	/// <param name="sizeExRate">画像サイズ(少数部分)(def = 0.75)</param>
	static void DrawTime(int x, int y, int frame, float size, float sizeExRate = FONT_EX_RATE)
	{
		auto time = FrameToTime(frame);
		auto& util = NumUtility::GetInstance();
		
		int add1 = static_cast<int>(NumUtility::SRC_SIZE_X * size);
		int add2 = static_cast<int>(NumUtility::SRC_SIZE_X * sizeExRate * size);

		int minX = x - add1 * 3;
		util.DrawNumberMore(minX, y, size, time.min, 2);
		int colonX = x - add1 * 2;
		util.DrawNumber(colonX, y, size, -1, L":");
		util.DrawNumberMore(x, y, size, time.sec, 2);
		int dotX = x + add1;
		util.DrawNumber(dotX, y, size, -1, L".");
		int decX = x + add1 + add2 * 2;
		int decY = y + static_cast<int>(NumUtility::SRC_SIZE_Y * 0.15f * size * sizeExRate);
		util.DrawNumberMore(decX, decY, size * sizeExRate, time.dec, 2);
	}
};