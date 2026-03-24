#pragma once
#include"Vec3.h"
#include"Quaternion.h"
using namespace std;

class Player;


class Camera
{
public:
	Camera();							// コンストラクタ.
	~Camera();							// デストラクタ.

	void Update(Vec3 LookPoint);	// 更新.
	void SetCamera(Vec3 LookPoint);
	void SetAimCamera(Vec3 LookPoint);
	void SetDebugCameraPoint();
	void DebagDraw();
	Vec3 cameraToPlayer(const Vec3& targetPos);
	
	const Vec3& GetPos() const { return m_pos; }

	void SetCameraPos(Vec3 LookPoint);
	void SetCameraFirstPersonPos(Vec3 LookPoint);
	void SetCameraThirdPersonPos(Vec3 LookPoint);
	float GetCameraAngle() const { return m_cameraAngle; }
	void SetUpVec(Vec3 upDir) { if (m_watchCount == 0) m_upVec = upDir; }
	Vec3 GetUpVec() const { return m_upVec; }
	void SetPlayerNormVec(Vec3 norm) { m_playerNormVec = norm; }
	void SetFrontVec(Vec3 front) { m_frontVec = front; }
	void SetBoost(bool boost) { m_isBoost = boost; }
	void SetCameraPoint(Vec3 pos) { if (m_watchCount == 0)m_cameraPoint = pos; }
	
	void WatchThis(Vec3 lookpoint,Vec3 cameraPos,Vec3 upVec);
	void NeutralUpdate(Vec3 LookPoint);
	void AimingUpdate(Vec3 LookPoint);
	//メンバ関数ポインタ
	using cameraState_t = void(Camera::*)(Vec3 lookpoint);
	cameraState_t m_cameraUpdate;
private:
	
	void WatchThisUpdate(Vec3 LookPoint);

private:
	int m_lightHandle = -1;
	int m_watchCount;
	bool m_isFirstPerson;
	bool m_isBoost;

	Vec3 m_frontVec;
	Quaternion m_myQ;
	Vec3 m_pos;			// ポジション.
	Vec3 m_cameraPoint;//カメラが移動してほしい位置
	Vec3 m_lookPoint;
	Vec3 m_postLookPointPos;
	float m_cameraAngle = -DX_PI_F / 2;
	float m_pitchAngle;
	Vec3 m_velocity;
	Vec3 m_playerNormVec;
	Vec3 m_upVec;
	/// <summary>
	/// プレイヤーから見たカメラの位置ベクトル
	/// </summary>
	Vec3 m_playerToCameraVec;
	Vec3 m_fowardVec;
};