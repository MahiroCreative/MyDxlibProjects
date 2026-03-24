#pragma once
#include "Camera.h"

class Gate;

class GateCamera : public Camera
{
public:
	GateCamera();
	virtual ~GateCamera();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="dir">方向</param>
	/// <param name="playerCameraFront">プレイヤーカメラの正面方向</param>
	void Update(const Vec3& dir, const Vec3& playerCameraFront);

	void SetBaseViewDir(const Vec3& dir) { m_baseViewDir = dir; }

private:
	Vec3 m_baseViewDir;
};

