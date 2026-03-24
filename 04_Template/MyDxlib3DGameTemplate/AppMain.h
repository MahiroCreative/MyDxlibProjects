#pragma once

/// <summary>
/// アプリケーションのメインループを管理するクラス.
/// シングルトンクラスとして実装
/// </summary>
class Application final
{
	// シングルトンパターンの実装
	//1.finalで継承禁止
	//2.コンストラクタをprivateにする
	//3.コピーコンストラクタと代入演算子を削除する
	//4.インスタンスを取得するための静的な関数を用意する
private:
	Application();//コンストラクタをprivateにする
	Application(const Application& app) = delete;//コピーコンストラクタを削除する
	void operator= (const Application& app) = delete;//代入演算子を削除する

public:
	static Application& GetInstance();//インスタンスを取得するための静的な関数を用意する

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
	void End();
};

