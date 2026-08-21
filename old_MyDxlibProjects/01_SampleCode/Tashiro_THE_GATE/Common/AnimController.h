#pragma once
#include <unordered_map>
#include <string>

class AnimController final
{
private:
	struct Info
	{
		std::wstring name;
		float speed = 0.0f;
	};

public:
	AnimController();
	~AnimController();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="path">IDテーブルがあるファイルパス</param>
	/// <param name="modelH">モデルハンドル</param>
	/// <param name="id">使用ID</param>
	void Init(const wchar_t* const path, int modelH, const wchar_t* const id);
	/// <summary>
	/// アニメーションの更新
	/// </summary>
	/// <param name="rate">速度割合</param>
	/// <returns>true:アニメーションが1ループした/ false: 更新中</returns>
	void Update(float rate = 1.0f);
	/// <summary>
	/// アニメーションの変更
	/// </summary>
	/// <param name="id">使用ID</param>
	/// <param name="isTopStart">true: アニメーションを初めに戻す/ false: 初めに戻さない</param>
	/// <param name="isStopEnd">true: アニメーション終了時に初めに戻らなくする/ false: 通常アニメーション</param>
	/// <param name="isBlendChange">true: アニメーションのブレンドを行う/ false: ブレンドを行わない</param>
	/// <param name="blendSpeed">ブレンドスピード(Def = 1/16)</param>
	/// <returns>true:成功/ false: 失敗or同じAnimIndex</returns>
	bool Change(const wchar_t* const id, bool isTopStart = true, bool isStopEnd = false, bool isBlendChange = true, float blendSpeed = -1.0f);
	/// <summary>
	/// 現在流れているアニメーションが一度でもループしているか
	/// </summary>
	/// <returns>true:ループしている / false:している</returns>
	bool IsLoop() const { return m_isLoop; }

private:
	void LoadIdTable(const wchar_t* const path);

	/// <summary>
	/// 通常アップデート
	/// </summary>
	/// <param name="rate">速度割合</param>
	void NormalUpdate(float rate);
	/// <summary>
	/// ブレンドアップデート
	/// </summary>
	/// <param name="rate">速度割合</param>
	void BlendUpdate(float rate);
	/// <summary>
	/// アニメーションの更新
	/// </summary>
	/// <param name="id">使用ID</param>
	/// <param name="attachIndex">アニメーションのアタッチ番号</param>
	/// <param name="rate">速度割合</param>
	/// <param name="isLoopCheck">ループ確認するか</param>
	/// <param name="isStopEnd">アニメーションを最後で停止させるか</param>
	void AnimUpdate(const std::wstring& id, int attachIndex, float rate, bool isLoopCheck, bool isStopEnd);

	/// <summary>
	/// ブレンドありでアニメーション変更
	/// </summary>
	/// <param name="animIndex">アニメ番号</param>
	/// <param name="blendSpeed">ブレンドスピード</param>
	void BlendChange(int animIndex, float blendSpeed);
	/// <summary>
	/// ブレンドなしでアニメーション変更
	/// </summary>
	/// <param name="animIndex">アニメ番号</param>
	void NoBlendChange(int animIndex);

private:
	// 更新メンバ関数
	using UpdateFunc_t = void(AnimController::*)(float);
	UpdateFunc_t m_updateFunc;

	// IDテーブル
	std::unordered_map<std::wstring, Info> m_idTable;

	// ID
	std::wstring m_nowId;
	std::wstring m_preId;

	// モデルハンドル
	int m_modelH;
	// 現在のアニメ番号
	int m_nowAnimIndex;
	// アタッチしている番号
	int m_nowAttachIndex;
	int m_preAttachIndex;

	// ブレンド率
	float m_blendRate;
	// ブレンドスピード
	float m_blendSpeed;

	// ループフラグ
	bool m_isLoop;
	// アニメーション最後で停止するか
	bool m_isStopEnd;
	bool m_isPreStopEnd;
};
