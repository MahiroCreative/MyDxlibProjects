#pragma once
#include <list>
#include <memory>

namespace MyEngine
{
	class Collidable;
}

/// <summary>
/// BGM,SEの再生を管理するシステム
/// </summary>
class SoundManager
{
private:
	struct SaveSeData
	{
		int handle;
		long long savePos;
		std::weak_ptr<MyEngine::Collidable> master;
	};

private:
	SoundManager();

	SoundManager(const SoundManager& snd) = delete;
	void operator= (const SoundManager& snd) = delete;

public:
	~SoundManager();

	static SoundManager& GetInstance();

	void SetSeCenter(const std::weak_ptr<MyEngine::Collidable>& center) { m_seCenter = center; }
	void ResetSeCenter() { m_seCenter.reset(); }

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 現在流れているSEを一時的に
	/// </summary>
	void StopNowPlaySe();
	/// <summary>
	/// 一時的に止めたSEを再び流す
	/// </summary>
	void RestartStopedSe();

	/// <summary>
	/// BGMを流す
	/// </summary>
	/// <param name="soundHnadle">BGMハンドル</param>
	/// <param name="isLoop">ループするか(デフォでtrue)</param>
	/// /// <param name="isSoundPosSave">再生場所の保存後の再生か</param>
	void PlayBgm(int soundHnadle = -1, bool isLoop = true, bool isSoundPosSave = false);

	/// <summary>
	/// Fade設定ありBGM
	/// </summary>
	/// <param name="soundHandle">BGMハンドル : -値だと現在流しているBGMをそのまま変更する</param>
	/// <param name="rate">音量パーセント</param>
	/// <param name="isLoop">ループするか</param>
	void PlayFadeBgm(int soundHandle = -1, float rate = 1.0f, bool isLoop = true);

	/// <summary>
	/// SEを流す
	/// </summary>
	/// <param name="seHandle">SEハンドル</param>
	/// <param name="isOption"></param>
	void PlaySe(int seHandle, bool isOption = false);
	/// <summary>
	/// 3D空間でSEを流す
	/// </summary>
	/// <param name="seHandle">SEハンドル</param>
	/// <param name="master">SEの発信源</param>
	/// <param name="isOption">オプションで鳴らしているものか</param>
	void PlaySe3D(int seHandle, const std::weak_ptr<MyEngine::Collidable> master, bool isOption = false);

	/// <summary>
	/// 音声の停止
	/// </summary>
	/// <param name="soundHandle">BGMハンドル</param>
	/// <param name="isSoundPosSave">再生場所の保存を行うか</param>
	void StopBGM(int soundHandle = -1, bool isSoundPosSave = false);
	
	/// <summary>
	/// 渡されたハンドルの音楽を停止
	/// </summary>
	/// <param name="handle">ハンドル</param>
	void Stop(int handle);

	/// <summary>
	/// BGMのボリュームを変更する
	/// </summary>
	/// <param name="volume">値</param>
	/// <param name="isApp">true: BGMに音量の変更をすぐ適用する /false: すぐ適用しない</param>
	void ChangeBgmVol(int volume, bool isApp = true);
	/// <summary>
	/// SEのボリュームを変更する
	/// </summary>
	/// <param name="volume">値</param>
	void ChangeSeVol(int volume);

	// Get関数
	int GetBgmVol() const { return m_bgmVolume; }
	int GetSeVol() const { return m_seVolume; }
	int GetMaxVol() const;
	bool IsNowPlay(int handle) const;
	/// <summary>
	/// BGMのボリュームの割合(0.0f ~ 1.0f)を取得
	/// </summary>
	/// <returns>BgmVolume</returns>
	float GetBgmVolRate() const;
	/// <summary>
	/// SEのボリュームの割合(0.0f ~ 1.0f)を取得
	/// </summary>
	/// <returns>SeVolum</returns>
	float GetSeVolRate() const;

private:

	/// <summary>
	/// 再生中のBGMの音量を変更する
	/// </summary>
	void SetBgmVol(float rate = 1.0f);
	/// <summary>
	/// 流しているSEの音量を変更する
	/// </summary>
	/// <param name="handle">SEハンドル</param>
	/// <param name="rate">割合</param>
	void ChangePlaySeVol(int handle, float rate);
	
private:
	std::weak_ptr<MyEngine::Collidable> m_seCenter;
	std::list<SaveSeData> m_saveSeList;

	// ボリューム調整
	int m_bgmVolume;
	int m_seVolume;

	// 現在再生中のBGM
	int m_nowPlayBgm;
	// 最後に鳴らしたSE
	int m_soundHandle;

	// 
	long long m_soundSavePos;

	bool m_isStop;
};

