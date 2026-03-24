#pragma once
#include <array>
#include <memory>

class Camera;
class GateManager;
enum class CameraKind
{
	PLAYER,
	GATE_A,
	GATE_B,
	GATE_A_FROM_B,
	GATE_B_FROM_A,
	MAX
};

class CameraManager
{
public:
	CameraManager();
	~CameraManager();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 終了処理
	/// </summary>
	void End();

	/// <summary>
	/// カメラの有効化
	/// </summary>
	/// <param name="kind">種類</param>
	void AppInfo(CameraKind kind);
	/// <summary>
	/// カメラに当たり判定をつける　USE_CAMERA_COLLIDABLEがないと有効ではない
	/// </summary>
	/// <param name="kind">種類</param>
	/// <param name="gateMgr">ゲートマネージャー</param>
	void OnEntity(CameraKind kind, const std::shared_ptr<GateManager>& gateMgr);

	/// <summary>
	/// カメラの取得
	/// </summary>
	/// <param name="kind">種類</param>
	/// <returns>カメラ</returns>
	std::shared_ptr<Camera> GetCamera(CameraKind kind) const { return m_cameras[static_cast<size_t>(kind)]; }

private:
	std::array<std::shared_ptr<Camera>, static_cast<size_t>(CameraKind::MAX)> m_cameras;
};

