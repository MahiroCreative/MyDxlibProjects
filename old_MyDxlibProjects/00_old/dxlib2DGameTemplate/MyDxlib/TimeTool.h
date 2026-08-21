#pragma once
//C++標準ライブラリ
#include<string>
#include<chrono>
#include<ctime>


namespace TimeTool
{
	using namespace std;

	/// <summary>
	/// 時刻を入れる構造体変数
	/// </summary>
	struct DataTime
	{
		int hour;
		int minute;
		int second;
	};

	/// <summary>
	/// 現在時刻をDataTime型で取得。
	/// </summary>
	/// <returns>DataTime(構造体)</returns>
	DataTime GetNowTimeData()
	{
		//変数
		DataTime time;

		// 現在時刻をsystem_clockを用いて取得
		auto now = std::chrono::system_clock::now();

		// 現在時刻をtime_t形式に変換
		std::time_t t = std::chrono::system_clock::to_time_t(now);

		//現在時刻を作成
		time.hour = (t / 3600 + 9) % 24;//時間
		time.minute = t / 60 % 60;//分
		time.second = t % 60;//秒

		//結果を返す
		return time;
	}

	/// <summary>
	/// 現在時刻をString型で返す。
	/// </summary>
	/// <returns>string</returns>
	string GetNowTimeString()
	{
		//現在時刻の取得
		DataTime time = GetNowTimeData();

		//現在時刻を文字列にまとめて返す
		return to_string(time.hour) + "," + to_string(time.minute) + "," + to_string(time.second);
	}
}
