#pragma once
#include <unordered_map>
#include <list>
#include <memory>

struct Vec3;
struct Quaternion;
class FileBase;

class EffekseerManager final
{
public:
	struct PlayInfo
	{
		std::shared_ptr<FileBase> file;
		std::list<int> handleList;
	};

private:
	EffekseerManager();
	~EffekseerManager();

	EffekseerManager(const EffekseerManager& app) = delete;
	void operator= (const EffekseerManager& app) = delete;

public:
	static EffekseerManager& GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="particle">画面に出すパーティクル数</param>
	void Init(int particle);
	/// <summary>
	/// 終了処理
	/// </summary>
	void End();
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() const;

	/// <summary>
	/// 再生
	/// </summary>
	/// <param name="id">再生したいエフェクトのファイルID</param>
	/// <returns>再生ハンドル</returns>
	int Play(const wchar_t* const id);
	/// <summary>
	/// 停止
	/// </summary>
	/// <param name="handle">停止させるエフェクトのハンドル</param>
	void Stop(int handle);
	/// <summary>
	/// 現在再生しているエフェクトを全て停止させる
	/// </summary>
	void AllStop();
	/// <summary>
	/// エフェクトに座標・回転を適用する
	/// </summary>
	/// <param name="handle">エフェクトハンドル</param>
	/// <param name="pos">座標</param>
	/// <param name="rot">回転</param>
	void SetInfo(int handle, const Vec3& pos, const Quaternion& rot);
	/// <summary>
	/// エフェクトに座標・回転・スケールを適用する
	/// </summary>
	/// <param name="handle">エフェクトハンドル</param>
	/// <param name="pos">座標</param>
	/// <param name="rot">回転</param>
	/// <param name="scale">スケール</param>
	void SetInfo(int handle, const Vec3& pos, const Quaternion& rot, const Vec3& scale);

private:
	std::unordered_map<const wchar_t*, PlayInfo> m_playInfoList;
};

