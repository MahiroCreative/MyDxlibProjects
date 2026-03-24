#pragma once
#include "Collidable.h"
#include "Geometry/Vec3.h"

// MEMO: 当たり判定ありにするなら先頭の_を外す
#define _USE_CAMERA_COLLIDABLE

struct CameraInfo
{
	// Tpsフラグ
	bool isTps = true;
	// near, far
	float n = 1.0f;
	float f = 100.0f;
	// 視野角
	float fov = 0.0f;
	// ターゲット位置
	Vec3 targetPos;
	// 正面方向
	Vec3 front;
	// 右方向
	Vec3 right;
	// 縦方向の回転具合
	float vertexAngle;
	// 見ている方向
	Vec3 look;

#ifndef USE_CAMERA_COLLIDABLE
	// 場所
	Vec3 cameraPos;
#endif
};

namespace MyEngine
{
	class ColliderSphere;
	struct CollideHitInfo;
}
struct Quaternion;
class GateManager;

#ifdef USE_CAMERA_COLLIDABLE
class Camera : public MyEngine::Collidable
#else
class Camera
#endif
{
public:
	Camera();
	virtual ~Camera();

	void Update();
#ifdef USE_CAMERA_COLLIDABLE
	virtual void OnCollideEnter(MyEngine::Collidable* colider, int colIndex, const MyEngine::CollideHitInfo& hitInfo) override;
	virtual void OnTriggerExit(MyEngine::Collidable* colider, int colIndex, const MyEngine::CollideHitInfo& hitInfo) override;
	void OnEntity();
	void SetGateManager(const std::shared_ptr<GateManager>& gateMgr) { m_gateMgr = gateMgr; }
#endif
	/// <summary>
	/// 情報の取得
	/// </summary>
	/// <returns>情報群</returns>
	const CameraInfo& GetInfo() const { return m_info; }
	
	/// <summary>
	/// near,farを設定
	/// </summary>
	/// <param name="n">near</param>
	/// <param name="f">far</param>
	void SetNearFar(float n, float f);
	/// <summary>
	/// 視野角の設定
	/// </summary>
	/// <param name="angle">角度</param>
	void SetFov(float angle);
	/// <summary>
	/// 初期位置の設定
	/// </summary>
	/// <param name="targetPos">カメラの注視点</param>
	void SetTargetPos(const Vec3& targetPos);

	/// <summary>
	/// カメラの有効化
	/// </summary>
	void AppInfo();

	/// <summary>
	/// カメラの場所を取得
	/// </summary>
	/// <returns>pos</returns>
	const Vec3& GetPos() const;
	/// <summary>
	/// TPSかFPSか
	/// </summary>
	/// <returns>true:TPS /false: FPS</returns>
	bool IsTps() const { return m_info.isTps; }

protected:
	/// <summary>
	/// カメラの距離を取得
	/// </summary>
	/// <returns></returns>
	float GetDistance() const;

protected:
#ifdef USE_CAMERA_COLLIDABLE
	std::shared_ptr<GateManager> m_gateMgr;
	MyEngine::ColliderSphere* m_collider;
#endif
	// 情報
	CameraInfo m_info;
};

