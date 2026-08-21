#pragma once
#include <random>

class Application final
{
private:
	Application();
	
	Application(const Application& app) = delete;
	void operator= (const Application& app) = delete;

public:
	static Application& GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <returns>true: 成功/ false:失敗</returns>
	bool Init();
	/// <summary>
	/// ゲームの実行
	/// </summary>
	void Run();
	/// <summary>
	/// 終了処理
	/// </summary>
	void Terminate();

	/// <summary>
	/// ゲームを終了させる
	/// </summary>
	void OnEnd() { m_isEnd = true; }

	/// <summary>
	/// 乱数の取得
	/// </summary>
	/// <returns>乱数</returns>
	int GetRand();
	/// <summary>
	/// 乱数の取得(整数で範囲制限)
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	int GetRand(int min, int max);
	/// <summary>
	/// 乱数の取得(実数で範囲制限)
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	float GetRand(float min, float max);

	/// <summary>
	/// 画面を明るく変更
	/// </summary>
	/// <param name="val">追加値</param>
	/// <returns>true: 変更した/ false: 変更していない</returns>
	bool AddScreenLight(int val);
	/// <summary>
	/// 画面を暗く変更
	/// </summary>
	/// <param name="val">減少値</param>
	/// <returns>true: 変更した/ false: 変更していない</returns>
	bool SubScreenLight(int val);
	int GetScreenLight() const { return m_screenLight; }

private:
	void InitManager();
	void EndManager();
#ifdef _DEBUG
	void ShowTime() const;
#endif

private:
	bool m_isEnd;
	bool m_isWindows;
	int m_screenLight;

	std::mt19937 m_mt;
#ifdef _DEBUG
	FILE* m_in;
	FILE* m_out;
#endif
};

