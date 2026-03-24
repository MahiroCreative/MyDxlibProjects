#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class SceneManager;
class FileBase;
class Input;

enum class SceneKind
{
	TITLE,	// タイトル
	STAGE_SELECT,	// ステージセレクト
	MAIN,	// プレイ画面
	CLEAR,	// クリア画面
	OPTION,	// オプション
	DEBUG,	// デバッグ
	MAX
};

/// <summary>
/// シーンの基底クラス
/// </summary>
class SceneBase
{
public:
	SceneBase(SceneKind kind);
	virtual ~SceneBase() {}

	/// <summary>
	/// シーンに入る時の初期化処理(非同期で行うもの)
	/// </summary>
	virtual void AsyncInit() abstract;
	/// <summary>
	/// シーンに入るときの初期化処理(同期処理で行うの)
	/// </summary>
	virtual void Init() abstract;
	/// <summary>
	/// シーンを抜けるときの処理
	/// </summary>
	virtual void End() abstract;
	/// <summary>
	/// 更新処理(同期)
	/// </summary>
	/// <param name="isFade">true:フェードしている /false:フェードしていない</param>
	virtual void Update(bool isFade) abstract;
	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() const abstract;

	/// <summary>
	/// 現在のシーンの種類
	/// </summary>
	/// <returns>シーンの種類</returns>
	SceneKind GetNowKind() const { return m_nowSceneKind; }

protected:
	SceneManager& m_scnMgr;
	std::unordered_map<std::wstring, std::shared_ptr<FileBase>> m_files;
	
private:
	SceneKind m_nowSceneKind;
};

